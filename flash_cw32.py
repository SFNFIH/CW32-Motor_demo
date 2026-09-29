#!/usr/bin/env python3
"""Flash CW32L012 via native pyOCD + WHXY CMSIS-Pack (cached outside the project).

Pack location (pyOCD / cmsis-pack-manager layout):
  ~/.local/share/cmsis-pack-manager/WHXY/CW32L012_DFP/1.0.2.pack

Install / refresh from vendor drop:
  python3 scripts/install_cw32_pack.py
"""
from __future__ import annotations

import argparse
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent
DEFAULT_ELF = ROOT / "build" / "Debug" / "cw32l012_blank.elf"
PACK_CACHE = (
    Path.home()
    / ".local"
    / "share"
    / "cmsis-pack-manager"
    / "WHXY"
    / "CW32L012_DFP"
    / "1.0.2.pack"
)
TARGET = "cw32l012c8"
PYOCD_VENV = Path("/home/tony/DAPLink/third_party/DAPLink/venv/bin/python")


def find_python() -> Path:
    if PYOCD_VENV.is_file():
        return PYOCD_VENV
    return Path(sys.executable)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Flash CW32L012 (pyOCD + WHXY DFP)")
    p.add_argument("elf", nargs="?", default=str(DEFAULT_ELF), help="ELF to flash")
    p.add_argument(
        "--pack",
        default=str(PACK_CACHE),
        help=f"DFP .pack path (default: {PACK_CACHE})",
    )
    p.add_argument("--probe", default="", help="Probe unique ID (empty = auto)")
    p.add_argument("--freq", type=int, default=100000, help="SWD clock Hz")
    p.add_argument(
        "--connect-mode",
        default="under-reset",
        choices=("under-reset", "halt", "pre-reset", "attach"),
    )
    p.add_argument("--erase-chip", action="store_true", help="Full chip erase before flash")
    p.add_argument("--connect-only", action="store_true", help="Only test SWD / target")
    p.add_argument("--retries", type=int, default=3, help="Retries on SWD failure")
    return p.parse_args()


def pyocd_cmd(args: argparse.Namespace, subcmd: str, extra: list[str]) -> list[str]:
    cmd = [
        str(find_python()),
        "-m",
        "pyocd",
        subcmd,
        f"--pack={Path(args.pack).expanduser().resolve()}",
        "-t",
        TARGET,
        "-f",
        str(args.freq),
        "-O",
        f"connect_mode={args.connect_mode}",
    ]
    if args.probe:
        cmd += ["-u", args.probe]
    cmd += extra
    return cmd


def run(cmd: list[str]) -> int:
    print("+", " ".join(cmd))
    return subprocess.call(cmd)


def main() -> int:
    args = parse_args()
    pack = Path(args.pack).expanduser()
    if not pack.is_file():
        print(f"Pack not found: {pack}", file=sys.stderr)
        print("Run: python3 scripts/install_cw32_pack.py", file=sys.stderr)
        return 1

    elf = Path(args.elf).expanduser().resolve()
    if not args.connect_only and not elf.is_file():
        print(f"ELF not found: {elf}", file=sys.stderr)
        return 1

    print(f"target={TARGET}")
    print(f"pack={pack.resolve()}")

    last = 1
    for attempt in range(1, args.retries + 1):
        if args.connect_only:
            cmd = pyocd_cmd(args, "cmd", ["-c", "status"])
        else:
            if args.erase_chip:
                rc = run(pyocd_cmd(args, "erase", ["--chip"]))
                if rc != 0:
                    print(f"erase attempt {attempt} failed (rc={rc})")
                    time.sleep(0.8)
                    last = rc
                    continue
            cmd = pyocd_cmd(args, "flash", [str(elf)])

        last = run(cmd)
        if last == 0:
            print("SWD OK." if args.connect_only else "DONE. Flashed via pyOCD pack algorithm.")
            return 0
        print(f"attempt {attempt}/{args.retries} failed (rc={last})")
        time.sleep(0.8)

    print(
        "SWD/flash failed.\n"
        "Check: close serial apps, power, SWD 1=SWDIO 2=SWCLK 3=+5V 4=GND",
        file=sys.stderr,
    )
    return last


if __name__ == "__main__":
    raise SystemExit(main())
