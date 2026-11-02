#!/usr/bin/env python3
"""Convert user PNGs to the indexed 8-bit files accepted by PS Vita LiveArea."""

from pathlib import Path
import struct
import sys
import zlib


def paeth(a, b, c):
    p = a + b - c
    pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
    return a if pa <= pb and pa <= pc else (b if pb <= pc else c)


def png_rows(path):
    data = Path(path).read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("not a PNG")
    cursor, compressed = 8, bytearray()
    width = height = color_type = None
    while cursor < len(data):
        length = struct.unpack(">I", data[cursor:cursor + 4])[0]
        kind = data[cursor + 4:cursor + 8]
        value = data[cursor + 8:cursor + 8 + length]
        cursor += length + 12
        if kind == b"IHDR":
            width, height, depth, color_type, _, _, interlace = struct.unpack(">IIBBBBB", value)
            if depth != 8 or color_type not in (2, 6) or interlace:
                raise ValueError("PNG must be non-interlaced 8-bit RGB or RGBA")
        elif kind == b"IDAT":
            compressed.extend(value)
        elif kind == b"IEND":
            break
    if width is None:
        raise ValueError("missing PNG header")
    channels = 4 if color_type == 6 else 3
    raw, stride = zlib.decompress(compressed), width * channels
    rows, offset, previous = [], 0, bytearray(stride)
    for _ in range(height):
        filter_type = raw[offset]
        current = bytearray(raw[offset + 1:offset + 1 + stride])
        offset += stride + 1
        for i, value in enumerate(current):
            left = current[i - channels] if i >= channels else 0
            up = previous[i]
            up_left = previous[i - channels] if i >= channels else 0
            if filter_type == 1:
                current[i] = (value + left) & 255
            elif filter_type == 2:
                current[i] = (value + up) & 255
            elif filter_type == 3:
                current[i] = (value + ((left + up) >> 1)) & 255
            elif filter_type == 4:
                current[i] = (value + paeth(left, up, up_left)) & 255
            elif filter_type != 0:
                raise ValueError("unsupported PNG filter")
        rows.append(current)
        previous = current
    return width, height, channels, rows


def resized_rgb(source, width, height):
    old_w, old_h, channels, rows = source
    pixels = []
    for y in range(height):
        row = rows[y * old_h // height]
        for x in range(width):
            offset = (x * old_w // width) * channels
            r, g, b = row[offset], row[offset + 1], row[offset + 2]
            if channels == 4:
                alpha = row[offset + 3]
                r, g, b = r * alpha // 255, g * alpha // 255, b * alpha // 255
            pixels.append((r, g, b))
    return pixels


def indexed(pixels):
    counts = [0] * 4096
    for r, g, b in pixels:
        counts[(r >> 4) << 8 | (g >> 4) << 4 | (b >> 4)] += 1
    bins = sorted(range(4096), key=lambda index: counts[index], reverse=True)[:256]
    palette = [((index >> 8) * 17, ((index >> 4) & 15) * 17, (index & 15) * 17) for index in bins]
    lookup = [0] * 4096
    for index in range(4096):
        r, g, b = (index >> 8) * 17, ((index >> 4) & 15) * 17, (index & 15) * 17
        lookup[index] = min(range(len(palette)), key=lambda i: (palette[i][0] - r) ** 2 + (palette[i][1] - g) ** 2 + (palette[i][2] - b) ** 2)
    return palette, bytes(lookup[(r >> 4) << 8 | (g >> 4) << 4 | (b >> 4)] for r, g, b in pixels)


def chunk(kind, value):
    return struct.pack(">I", len(value)) + kind + value + struct.pack(">I", zlib.crc32(kind + value) & 0xffffffff)


def write_png(path, width, height, palette, indices):
    rows = b"".join(b"\0" + indices[row * width:(row + 1) * width] for row in range(height))
    palette_bytes = bytes(component for color in palette for component in color)
    output = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 3, 0, 0, 0))
    output += chunk(b"PLTE", palette_bytes) + chunk(b"IDAT", zlib.compress(rows, 9)) + chunk(b"IEND", b"")
    Path(path).write_bytes(output)


if __name__ == "__main__":
    if len(sys.argv) != 5:
        raise SystemExit("usage: prepare_livearea.py SOURCE TARGET WIDTH HEIGHT")
    target, width, height = Path(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])
    target.parent.mkdir(parents=True, exist_ok=True)
    palette, indices = indexed(resized_rgb(png_rows(sys.argv[1]), width, height))
    write_png(target, width, height, palette, indices)
