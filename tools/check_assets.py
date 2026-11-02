#!/usr/bin/env python3

from pathlib import Path
from struct import unpack
from hashlib import sha256

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "assets"

EXPECTED = {
    "banners.png": (128, 256),
    "arcs1.png": (32, 32),
    "arcs2.png": (64, 64),
    "dashboard.png": (128, 32),
    "font1x.png": (1024, 8),
    "avatars.png": (128, 32),
    "chrome.png": (128, 64),
    "buffs.png": (128, 16),
    "items.png": (128, 256),
    "status_psvita.png": (128, 64),

    "tiles0.png": (256, 64),
    "warrior.png": (256, 128),
    "mage.png": (256, 128),
    "rogue.png": (256, 128),
    "ranger.png": (256, 128),
    "rat.png": (256, 32),
}

EXPECTED_FONT1X_SHA256 = (
    "72e5df90b86959f9bb32d58f6350a351923ee003d432030d50aa110ac63f53b8"
)

def png_size(path: Path):
    data = path.read_bytes()

    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("not a PNG")

    if data[12:16] != b"IHDR":
        raise ValueError("missing IHDR")

    return unpack(">II", data[16:24])

failed = False

for name, expected in EXPECTED.items():
    path = ASSETS / name

    if not path.exists():
        print(f"[MISSING] {name}")
        failed = True
        continue

    try:
        actual = png_size(path)
    except Exception as exc:
        print(f"[BAD] {name}: {exc}")
        failed = True
        continue

    if actual != expected:
        print(f"[BAD SIZE] {name}: {actual}, expected {expected}")
        failed = True
    else:
        print(f"[OK] {name}: {actual[0]}x{actual[1]}")

font_hash = sha256((ASSETS / "font1x.png").read_bytes()).hexdigest()
if font_hash != EXPECTED_FONT1X_SHA256:
    print("[BAD FONT] font1x.png has an incompatible glyph layout")
    failed = True
else:
    print("[OK FONT] font1x.png glyph layout verified")

for name in ("theme.mp3", "game.mp3", "surface.mp3"):
    path = ASSETS / name
    if not path.exists() or path.stat().st_size < 1024:
        print(f"[MISSING/BAD AUDIO] {name}")
        failed = True
    else:
        print(f"[OK AUDIO] {name}: {path.stat().st_size} bytes")

sounds = sorted(ASSETS.glob("snd_*.mp3"))
if len(sounds) < 46 or any(path.stat().st_size < 256 for path in sounds):
    print(f"[BAD SOUND SET] found {len(sounds)}, expected at least 46 valid files")
    failed = True
else:
    print(f"[OK SOUND SET] {len(sounds)} effects")

raise SystemExit(1 if failed else 0)
