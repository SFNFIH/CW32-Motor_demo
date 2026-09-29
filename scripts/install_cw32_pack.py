#!/usr/bin/env python3
"""Install WHXY.CW32L012_DFP into pyOCD's local cmsis-pack-manager cache.

Destination (standard layout):
  ~/.local/share/cmsis-pack-manager/WHXY/CW32L012_DFP/1.0.2.pack
  ~/.local/share/cmsis-pack-manager/WHXY.CW32L012_DFP.1.0.2.pdsc

Note: WHXY is not on the public pack index, so `pyocd pack install CW32*`
will not find it. Point pyocd.yml / --pack at the path above instead.
"""
from __future__ import annotations

import argparse
import shutil
import zipfile
from pathlib import Path

VERSION = "1.0.2"
VENDOR = "WHXY"
PACK_NAME = "CW32L012_DFP"
DEFAULT_SRC = Path("/home/tony/CW32/IDEsupport/MDK") / f"{VENDOR}.{PACK_NAME}.{VERSION}.pack"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "src",
        nargs="?",
        default=str(DEFAULT_SRC),
        help=f"Source .pack (default: {DEFAULT_SRC})",
    )
    args = ap.parse_args()
    src = Path(args.src).expanduser().resolve()
    if not src.is_file():
        print(f"Source pack not found: {src}")
        return 1

    cache = Path.home() / ".local" / "share" / "cmsis-pack-manager"
    dest_dir = cache / VENDOR / PACK_NAME
    dest_pack = dest_dir / f"{VERSION}.pack"
    dest_pdsc = cache / f"{VENDOR}.{PACK_NAME}.{VERSION}.pdsc"
    dest_dir.mkdir(parents=True, exist_ok=True)

    shutil.copy2(src, dest_pack)
    # Extract pdsc (+ prefer .svd in debug line if present)
    with zipfile.ZipFile(dest_pack) as z:
        pdsc_name = next(n for n in z.namelist() if n.endswith(".pdsc"))
        data = z.read(pdsc_name).decode("utf-8")
    data = data.replace('svd="SVD/CW32L012.sfr"', 'svd="SVD/CW32L012.svd"')
    dest_pdsc.write_text(data, encoding="utf-8")

    # Optional mirror under ~/CW32/packs
    mirror = Path("/home/tony/CW32/packs")
    mirror.mkdir(parents=True, exist_ok=True)
    shutil.copy2(dest_pack, mirror / f"{VENDOR}.{PACK_NAME}.{VERSION}.pack")

    print(f"Installed:\n  {dest_pack}\n  {dest_pdsc}\n  {mirror / f'{VENDOR}.{PACK_NAME}.{VERSION}.pack'}")
    print("Use in pyocd.yml:\n  pack:\n    -", dest_pack)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
