#!/usr/bin/env python3
"""Closed-loop motor debug via PyOCD: flash, poke g_dbg.cmd, sample g_dbg → NDJSON."""
from __future__ import annotations

import argparse
import json
import struct
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LOG_PATH = ROOT / ".cursor" / "debug-532fc6.log"
DEFAULT_ELF = ROOT / "build" / "Debug" / "cw32l012_blank.elf"
PYOCD = Path("/home/tony/DAPLink/third_party/DAPLink/venv/bin/pyocd")

# Must match BSP_DebugSnap.h packed layout
SNAP_FMT = "<IIHHHHHHHHHhHHiiBBHI"
SNAP_SIZE = struct.calcsize(SNAP_FMT)
MAGIC = 0x532FC60D
CMD_START, CMD_STOP, CMD_TOGGLE_DIR = 1, 2, 3


def nm_addr(elf: Path, symbol: str) -> int:
    out = subprocess.check_output(
        ["arm-none-eabi-nm", str(elf)], text=True, errors="replace"
    )
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[-1] == symbol:
            return int(parts[0], 16)
    raise SystemExit(f"symbol {symbol!r} not found in {elf}")


def append_log(payload: dict) -> None:
    LOG_PATH.parent.mkdir(parents=True, exist_ok=True)
    payload.setdefault("sessionId", "532fc6")
    payload.setdefault("timestamp", int(time.time() * 1000))
    with LOG_PATH.open("a", encoding="utf-8") as f:
        f.write(json.dumps(payload, ensure_ascii=False) + "\n")


def unpack_snap(raw: bytes) -> dict:
    (
        magic,
        seq,
        state,
        step,
        bemf,
        mid,
        duty,
        pot,
        zc,
        miss,
        period,
        direction,
        as_raw,
        as_status,
        as_cum,
        as_rpm_x10,
        as_ok,
        motor_on,
        reserved,
        cmd,
    ) = struct.unpack(SNAP_FMT, raw)
    return {
        "magic": magic,
        "seq": seq,
        "state": state,
        "step": step,
        "bemf": bemf,
        "mid": mid,
        "duty": duty,
        "pot": pot,
        "zc": zc,
        "miss": miss,
        "period": period,
        "dir": direction,
        "as_raw": as_raw,
        "as_status": as_status,
        "as_cum": as_cum,
        "as_rpm": as_rpm_x10 / 10.0,
        "as_ok": as_ok,
        "motor_on": motor_on,
        "i2c_err": reserved & 0x0F,
        "idle_pins": (reserved >> 4) & 0x0F,
        "found_addr": (reserved >> 8) & 0xFF,
        "cmd": cmd,
    }


def read_snap(target, addr: int, retries: int = 4) -> dict:
    """Halt briefly so SWD stays reliable while motor PWM runs."""
    from pyocd.core.exceptions import TransferError

    last: Exception | None = None
    for attempt in range(retries):
        try:
            try:
                target.halt()
            except TransferError:
                time.sleep(0.05)
                target.reset_and_halt()
            raw = bytes(target.read_memory_block8(addr, SNAP_SIZE))
            target.resume()
            return unpack_snap(raw)
        except TransferError as exc:
            last = exc
            time.sleep(0.05 * (attempt + 1))
            try:
                target.reset_and_halt()
                target.resume()
            except Exception:
                pass
    raise RuntimeError(f"SWD read failed after {retries} tries: {last}")


def poke_u32(target, addr: int, value: int) -> None:
    from pyocd.core.exceptions import TransferError

    for _ in range(3):
        try:
            target.halt()
            target.write_memory(addr, value)
            target.resume()
            return
        except TransferError:
            time.sleep(0.05)
    raise RuntimeError(f"poke_u32 failed @ {addr:#x}")


def poke_u16(target, addr: int, value: int) -> None:
    from pyocd.core.exceptions import TransferError

    for _ in range(3):
        try:
            target.halt()
            target.write16(addr, value & 0xFFFF)
            target.resume()
            return
        except TransferError:
            time.sleep(0.05)
    raise RuntimeError(f"poke_u16 failed @ {addr:#x}")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--elf", type=Path, default=DEFAULT_ELF)
    ap.add_argument("--seconds", type=float, default=10.0)
    ap.add_argument("--hz", type=float, default=25.0)
    ap.add_argument("--run-id", default="pyocd-loop")
    ap.add_argument("--flash", action="store_true", help="flash ELF before run")
    ap.add_argument("--force-duty", type=int, default=0, help="override pot duty (CCR counts)")
    ap.add_argument("--start", action="store_true", help="send DBG_CMD_START")
    ap.add_argument("--stop-at-end", action="store_true", default=True)
    ap.add_argument("--no-stop-at-end", dest="stop_at_end", action="store_false")
    args = ap.parse_args()

    if not args.elf.is_file():
        raise SystemExit(f"missing ELF: {args.elf}")
    if not PYOCD.is_file():
        raise SystemExit(f"missing pyocd: {PYOCD}")

    # Import from DAPLink venv
    venv_site = PYOCD.parent.parent / "lib"
    for p in venv_site.glob("python*/site-packages"):
        sys.path.insert(0, str(p))
        break

    from pyocd.core.helpers import ConnectHelper
    from pyocd.flash.file_programmer import FileProgrammer

    g_dbg = nm_addr(args.elf, "g_dbg")
    g_force = nm_addr(args.elf, "g_force_duty")
    cmd_off = SNAP_SIZE - 4  # volatile uint32_t cmd at end

    append_log(
        {
            "runId": args.run_id,
            "hypothesisId": "setup",
            "location": "pyocd_loop_debug.py",
            "message": "attach",
            "data": {
                "g_dbg": hex(g_dbg),
                "g_force_duty": hex(g_force),
                "snap_size": SNAP_SIZE,
                "elf": str(args.elf),
            },
        }
    )

    options = {
        "frequency": 100000,
        "connect_mode": "under-reset",
        "reset_type": "hw",
        "pack": [
            "/home/tony/.local/share/cmsis-pack-manager/WHXY/CW32L012_DFP/1.0.2.pack"
        ],
    }

    with ConnectHelper.session_with_chosen_probe(
        target_override="cw32l012c8",
        options=options,
    ) as session:
        target = session.board.target
        if args.flash:
            FileProgrammer(session).program(str(args.elf))
            target.reset_and_halt()
            target.resume()
            time.sleep(0.4)

        # live reads without halt when possible
        target.resume()
        time.sleep(0.2)

        if args.force_duty > 0:
            poke_u16(target, g_force, int(args.force_duty))

        snap0 = read_snap(target, g_dbg)
        append_log(
            {
                "runId": args.run_id,
                "hypothesisId": "H-as5600",
                "location": "pyocd_loop_debug.py:probe",
                "message": "first_snap",
                "data": snap0,
            }
        )
        if snap0["magic"] != MAGIC:
            append_log(
                {
                    "runId": args.run_id,
                    "hypothesisId": "setup",
                    "location": "pyocd_loop_debug.py",
                    "message": "bad_magic",
                    "data": snap0,
                }
            )
            print("bad magic — is firmware flashed?", snap0)
            return 2

        if args.start:
            poke_u32(target, g_dbg + cmd_off, CMD_START)
            time.sleep(0.3)

        t_end = time.time() + args.seconds
        period = 1.0 / max(args.hz, 1.0)
        n = 0
        last_seq = -1
        while time.time() < t_end:
            try:
                snap = read_snap(target, g_dbg)
            except RuntimeError as exc:
                append_log(
                    {
                        "runId": args.run_id,
                        "hypothesisId": "H-swd",
                        "location": "pyocd_loop_debug.py:sample",
                        "message": "swd_fail",
                        "data": {"err": str(exc)},
                    }
                )
                time.sleep(0.2)
                continue
            if snap["seq"] != last_seq:
                last_seq = snap["seq"]
                n += 1
                hyp = []
                if snap["state"] != 3:
                    hyp.append("H-state")
                if snap["bemf"] < 20:
                    hyp.append("H-bemf")
                if snap["as_ok"] == 0:
                    hyp.append("H-as5600")
                if snap["motor_on"] and abs(snap["as_rpm"]) < 5 and snap["state"] >= 2:
                    hyp.append("H-stall")
                append_log(
                    {
                        "runId": args.run_id,
                        "hypothesisId": ",".join(hyp) or "ok",
                        "location": "pyocd_loop_debug.py:sample",
                        "message": "snap",
                        "data": snap,
                    }
                )
            time.sleep(period)

        if args.stop_at_end:
            try:
                poke_u32(target, g_dbg + cmd_off, CMD_STOP)
                poke_u16(target, g_force, 0)
                time.sleep(0.2)
            except RuntimeError:
                pass

        try:
            snap_f = read_snap(target, g_dbg)
        except RuntimeError as exc:
            snap_f = {"error": str(exc)}
        append_log(
            {
                "runId": args.run_id,
                "hypothesisId": "summary",
                "location": "pyocd_loop_debug.py",
                "message": "done",
                "data": {"samples": n, "final": snap_f},
            }
        )
        if "error" in snap_f:
            print(f"samples={n} final_read_failed={snap_f['error']}")
        else:
            print(
                f"samples={n} state={snap_f['state']} bemf={snap_f['bemf']} "
                f"duty={snap_f['duty']} as_ok={snap_f['as_ok']} "
                f"as_err={snap_f.get('i2c_err')} idle={snap_f.get('idle_pins')} "
                f"found=0x{snap_f.get('found_addr', 0):02x} "
                f"rpm={snap_f['as_rpm']:.1f} as_raw={snap_f['as_raw']} "
                f"zc={snap_f['zc']} miss={snap_f['miss']}"
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
