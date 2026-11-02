#!/usr/bin/env python3
"""Create PSVita-compatible indexed PNG copies of the LiveArea artwork."""

from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = ROOT / "sce_sys"
OUTPUT_ROOT = SOURCE_ROOT / "vita"
FILES = (
    Path("icon0.png"),
    Path("pic0.png"),
    Path("livearea/contents/bg0.png"),
    Path("livearea/contents/startup.png"),
)


def convert(source: Path, output: Path, flatten_alpha: bool = False) -> None:
    output.parent.mkdir(parents=True, exist_ok=True)
    with Image.open(source) as image:
        rgba = image.convert("RGBA")
        if flatten_alpha:
            opaque = Image.new("RGBA", rgba.size, (0, 0, 0, 255))
            opaque.alpha_composite(rgba)
            indexed = opaque.convert("RGB").quantize(
                colors=256,
                method=Image.Quantize.MEDIANCUT,
                dither=Image.Dither.FLOYDSTEINBERG,
            )
        else:
            indexed = rgba.quantize(
                colors=255,
                method=Image.Quantize.FASTOCTREE,
                dither=Image.Dither.FLOYDSTEINBERG,
            )
        indexed.save(output, format="PNG", optimize=True)


def main() -> None:
    for relative_path in FILES:
        source = SOURCE_ROOT / relative_path
        if not source.is_file():
            raise FileNotFoundError(source)
        output = OUTPUT_ROOT / relative_path
        # The Vita installer is stricter than common PNG readers: palette transparency
        # chunks (tRNS) can make an otherwise valid LiveArea image reject installation.
        # Flatten all supplied artwork before indexing so the generated files are opaque.
        convert(source, output, flatten_alpha=True)
        print(f"{source.relative_to(ROOT)} -> {output.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
