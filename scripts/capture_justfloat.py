#!/usr/bin/env python3
"""Capture JustFloat telemetry from CW32 into debug NDJSON log."""
from __future__ import annotations

import argparse
import json
import struct
import time
from pathlib import Path

TAIL = b"\x00\x00\x80\x7f"
LOG = Path("/home/tony/CW32/cw32l012_blank/.cursor/debug-532fc6.log")
N_CH = 8
FRAME = 4 * N_CH + 4


def parse_frames(buf: bytearray):
    frames = []
    while True:
        i = buf.find(TAIL)
        if i < 0:
            break
        start = i - 4 * N_CH
        if start < 0:
            del buf[: i + 4]
            continue
        raw = bytes(buf[start:i])
        del buf[: i + 4]
        vals = struct.unpack("<" + "f" * N_CH, raw)
        frames.append(vals)
    return frames


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", default="/dev/ttyACM0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--seconds", type=float, default=20.0)
    ap.add_argument("--run-id", default="pre-fix")
    args = ap.parse_args()

    import serial

    print(f"Open {args.port} @ {args.baud}, capture {args.seconds}s ...")
    print("请在倒计时期间: 单击启动 → 拧电位器 → 可选换向 → 停机")
    ser = serial.Serial(args.port, args.baud, timeout=0.05)
    ser.reset_input_buffer()
    buf = bytearray()
    t0 = time.time()
    n = 0
    pot_min = 1e9
    pot_max = -1e9
    state_seen = set()

    with LOG.open("a", encoding="utf-8") as fp:
        fp.write(
            json.dumps(
                {
                    "sessionId": "532fc6",
                    "runId": args.run_id,
                    "hypothesisId": "setup",
                    "location": "capture_justfloat.py",
                    "message": "capture_start",
                    "data": {"port": args.port, "baud": args.baud},
                    "timestamp": int(time.time() * 1000),
                }
            )
            + "\n"
        )
        while time.time() - t0 < args.seconds:
            chunk = ser.read(256)
            if chunk:
                buf.extend(chunk)
                for f in parse_frames(buf):
                    state, step, bemf, duty, pot, direc, mid, miss = f
                    state_seen.add(int(state))
                    pot_min = min(pot_min, pot)
                    pot_max = max(pot_max, pot)
                    n += 1
                    # downsample: log every 5th frame (~100ms)
                    if n % 5 == 0:
                        hyp = []
                        if int(state) != 3:
                            hyp.append("A")
                        if pot_max - pot_min < 50:
                            hyp.append("B")
                        if miss > 5:
                            hyp.append("C")
                        if mid < 100 or mid > 2000:
                            hyp.append("D")
                        fp.write(
                            json.dumps(
                                {
                                    "sessionId": "532fc6",
                                    "runId": args.run_id,
                                    "hypothesisId": ",".join(hyp) or "ok",
                                    "location": "capture_justfloat.py:frame",
                                    "message": "telemetry",
                                    "data": {
                                        "state": int(state),
                                        "step": int(step),
                                        "bemf": round(bemf, 1),
                                        "duty_cmd": round(duty, 1),
                                        "pot": round(pot, 1),
                                        "dir": int(direc),
                                        "mid": round(mid, 1),
                                        "zc": round(miss, 1),
                                        "n": n,
                                    },
                                    "timestamp": int(time.time() * 1000),
                                }
                            )
                            + "\n"
                        )
        fp.write(
            json.dumps(
                {
                    "sessionId": "532fc6",
                    "runId": args.run_id,
                    "hypothesisId": "summary",
                    "location": "capture_justfloat.py",
                    "message": "capture_end",
                    "data": {
                        "frames": n,
                        "states": sorted(state_seen),
                        "pot_min": pot_min if pot_min < 1e8 else None,
                        "pot_max": pot_max if pot_max > -1e8 else None,
                        "pot_span": (pot_max - pot_min) if pot_max > -1e8 else None,
                    },
                    "timestamp": int(time.time() * 1000),
                }
            )
            + "\n"
        )
    ser.close()
    print(f"frames={n} states={sorted(state_seen)} pot=[{pot_min},{pot_max}] -> {LOG}")


if __name__ == "__main__":
    main()
