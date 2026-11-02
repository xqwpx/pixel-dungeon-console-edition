#!/usr/bin/env python3

"""
Optional helper.

Usage:
    python3 tools/import_extracted_assets.py /path/to/extracted/apk/assets
"""

from pathlib import Path
import shutil
import sys

if len(sys.argv) != 2:
    print("Usage: import_extracted_assets.py /path/to/assets")
    raise SystemExit(2)

source = Path(sys.argv[1])
root = Path(__file__).resolve().parents[1]
target = root / "assets"

names = [
    "banners.png",
    "arcs1.png",
    "arcs2.png",
    "dashboard.png",
    "font1x.png",
    "avatars.png",
    "chrome.png",
    "buffs.png",
    "items.png",
    "tiles0.png",
    "warrior.png",
    "mage.png",
    "rogue.png",
    "ranger.png",
    "rat.png",
]

for name in names:
    src = source / name

    if not src.exists():
        print(f"[SKIP] {src}")
        continue

    shutil.copy2(src, target / name)
    print(f"[COPY] {name}")
