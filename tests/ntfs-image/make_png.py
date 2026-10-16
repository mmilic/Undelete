"""Writes a small valid PNG (a colour gradient) - a real image for the preview test.

usage: make_png.py <width> <height> <output file>
"""
import struct
import sys
import zlib

width, height, out = int(sys.argv[1]), int(sys.argv[2]), sys.argv[3]


def chunk(kind, data):
    body = kind + data
    return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)


rows = bytearray()
for y in range(height):
    rows.append(0)  # filter type: none
    for x in range(width):
        rows += bytes((x * 255 // width, y * 255 // height, 128))  # RGB
rows = bytes(rows)
png = (b"\x89PNG\r\n\x1a\n"
       + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))  # 8-bit RGB
       + chunk(b"IDAT", zlib.compress(rows))
       + chunk(b"IEND", b""))

with open(out, "wb") as f:
    f.write(png)
