"""Writes a Recycle Bin $I file, the companion of a $R file that holds the original path.

usage: make_recycle_info.py <version 1|2> <original path> <original size> <output file>

  version 1 (Vista - 8.1): 8 version, 8 size, 8 deletion time, path as 260 WCHARs (zero padded)
  version 2 (Windows 10+): 8 version, 8 size, 8 deletion time, 4 path length in WCHARs (incl. the
                           terminating zero), path
"""
import struct
import sys

version, path, size, out = int(sys.argv[1]), sys.argv[2], int(sys.argv[3]), sys.argv[4]
deleted_at = 133700000000000000  # a FILETIME in 2024 - the engine does not use it

header = struct.pack("<qqq", version, size, deleted_at)
name = (path + "\0").encode("utf-16-le")
if version == 1:
    body = name.ljust(260 * 2, b"\0")
elif version == 2:
    body = struct.pack("<I", len(path) + 1) + name
else:
    sys.exit("version must be 1 or 2")

with open(out, "wb") as f:
    f.write(header + body)
