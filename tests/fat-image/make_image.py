"""Builds the FAT test images: FAT12, FAT16 and FAT32 volumes holding deleted files.

usage: make_image.py <output dir>

Writes fat12.img, fat16.img, fat32.img and for each an expected list (<image>.expected.tsv): one line per
deleted test file - the name it must be recovered under, the SHA-256 of its data ("-": don't compare),
the recoverability, the folder and the content check result ("-": don't check) the engine must report.

The volumes are formatted and filled by pyfatfs (an independent FAT implementation). pyfatfs deletes files by
rewriting the whole directory, which leaves no deleted entries behind, so the deleting is done here, the
way Windows does it: the first byte of the 8.3 entry and of its long name entries becomes 0xE5 and the
cluster chain is freed in every FAT. A folder's files are deleted before the folder.
"""
import hashlib
import os
import random
import struct
import subprocess
import sys
import tempfile

from pyfatfs.PyFat import PyFat
from pyfatfs.PyFatFS import PyFatFS

HERE = os.path.dirname(os.path.abspath(__file__))
NTFS_TEST = os.path.join(HERE, "..", "ntfs-image")  # the PNG and $I file writers are shared with the NTFS test


def helper_output(script, *args):
    """runs one of the NTFS test's writers and returns the file it wrote"""
    with tempfile.TemporaryDirectory() as tmp:
        out = os.path.join(tmp, "out")
        subprocess.run([sys.executable, "-I", os.path.join(NTFS_TEST, script), *map(str, args), out], check=True)
        with open(out, "rb") as f:
            return f.read()


class RawFat:
    """just enough FAT to find a file by its path and delete it like Windows does"""

    def __init__(self, path):
        self.path = path
        with open(path, "rb") as f:
            self.d = bytearray(f.read())
        bps, spc, rsvd, nfats, rootent, tot16, _, fatsz16 = struct.unpack_from("<HBHBHHBH", self.d, 11)
        tot32, fatsz32 = struct.unpack_from("<II", self.d, 32)
        fatsz = fatsz16 or fatsz32
        root_secs = (rootent * 32 + bps - 1) // bps
        clusters = ((tot16 or tot32) - (rsvd + nfats * fatsz + root_secs)) // spc
        self.type = 12 if clusters < 4085 else (16 if clusters < 65525 else 32)
        self.bpc = bps * spc
        self.fat_off = rsvd * bps
        self.fat_bytes = fatsz * bps
        self.nfats = nfats
        self.root_off = self.fat_off + nfats * self.fat_bytes
        self.root_bytes = root_secs * bps
        self.data_off = self.root_off + self.root_bytes
        self.root_cluster = struct.unpack_from("<I", self.d, 44)[0] if self.type == 32 else 0
        self.eoc = {12: 0xFF8, 16: 0xFFF8, 32: 0x0FFFFFF8}[self.type]

    def get(self, c):
        o = self.fat_off
        if self.type == 12:
            w = struct.unpack_from("<H", self.d, o + c + c // 2)[0]
            return w >> 4 if c & 1 else w & 0xFFF
        if self.type == 16:
            return struct.unpack_from("<H", self.d, o + c * 2)[0]
        return struct.unpack_from("<I", self.d, o + c * 4)[0] & 0x0FFFFFFF

    def set(self, c, v):
        for k in range(self.nfats):
            o = self.fat_off + k * self.fat_bytes
            if self.type == 12:
                p = o + c + c // 2
                w = struct.unpack_from("<H", self.d, p)[0]
                w = (w & 0x000F) | (v << 4) if c & 1 else (w & 0xF000) | v
                struct.pack_into("<H", self.d, p, w)
            elif self.type == 16:
                struct.pack_into("<H", self.d, o + c * 2, v)
            else:
                p = o + c * 4
                old = struct.unpack_from("<I", self.d, p)[0]
                struct.pack_into("<I", self.d, p, (old & 0xF0000000) | v)

    def chain(self, c):
        while 2 <= c < self.eoc:
            yield c
            c = self.get(c)

    def slots(self, cluster):
        if cluster == 0:
            return range(self.root_off, self.root_off + self.root_bytes, 32)
        return [self.data_off + (c - 2) * self.bpc + i for c in self.chain(cluster) for i in range(0, self.bpc, 32)]

    def entries(self, cluster):
        """the live entries of a folder: (name, entry offset, long name entry offsets, first cluster, attributes)"""
        lfn = []
        for off in self.slots(cluster):
            e = self.d[off:off + 32]
            if e[0] == 0:
                return
            if e[0] == 0xE5:
                lfn = []
                continue
            if e[11] == 0x0F:
                lfn.append(off)
                continue
            if lfn:  # stored last part first
                raw = b"".join(self.d[o + 1:o + 11] + self.d[o + 14:o + 26] + self.d[o + 28:o + 32] for o in reversed(lfn))
                name = raw.decode("utf-16-le").split("\0")[0]
            else:
                base, ext = bytes(e[0:8]).decode("cp437").rstrip(), bytes(e[8:11]).decode("cp437").rstrip()
                base = base.lower() if e[12] & 0x08 else base
                ext = ext.lower() if e[12] & 0x10 else ext
                name = base + ("." + ext if ext else "")
            cl = struct.unpack_from("<H", e, 26)[0] | (struct.unpack_from("<H", e, 20)[0] << 16 if self.type == 32 else 0)
            yield name, off, lfn, cl, e[11]
            lfn = []

    def find(self, path):
        cluster, found = self.root_cluster, None
        for part in path.strip("/").split("/"):
            found = next((x for x in self.entries(cluster) if x[0] == part), None)
            if not found:
                raise FileNotFoundError(path)
            cluster = found[3]
        return found

    def delete(self, path):
        name, off, lfn, cluster, attr = self.find(path)
        if attr & 0x10:  # a folder: its files first, like Windows
            for child in list(self.entries(cluster)):
                if child[0] not in (".", ".."):
                    self.delete(path.rstrip("/") + "/" + child[0])
        for o in [off] + lfn:
            self.d[o] = 0xE5
        for c in list(self.chain(cluster)):
            self.set(c, 0)

    def save(self):
        with open(self.path, "wb") as f:
            f.write(self.d)


def used_clusters(fs):
    fat = fs.fs.fat
    return [c for c in range(2, len(fat)) if fat[c]]


def build(out_dir, name, fat_type, size):
    img = os.path.join(out_dir, name + ".img")
    if os.path.exists(img):
        os.remove(img)
    open(img, "wb").close()
    pf = PyFat()
    pf.mkfs(img, fat_type, size=size, label="UNDELTEST")
    pf.close()

    rnd = random.Random(name)  # the same image every time
    expected = []  # (name, sha256 or "-", recoverability, folder, content or "-")
    files = {}  # path -> data, for the checksums

    def write(fs, path, data):
        fs.writebytes(path, data)
        files[path] = data

    def expect(path, name=None, recov="Good", folder="\\", content="-", check_data=True):
        name = name or path.rsplit("/", 1)[-1]
        sha = hashlib.sha256(files[path]).hexdigest() if check_data else "-"
        expected.append((name, sha, recov, folder, content))

    fs = PyFatFS(img)
    bpc = fs.fs.bytes_per_cluster
    fs.makedir("/Keep")  # gets a file later, after the deletes - the other folders must not be written then

    # 20 live fillers of 8 clusters, then every other one freed (pyfatfs leaves no trace of them) -> 8-cluster holes
    for i in range(20):
        fs.writebytes(f"/FILL{i:02d}.BIN", rnd.randbytes(8 * bpc))
    for i in range(1, 20, 2):
        fs.remove(f"/FILL{i:02d}.BIN")

    # fills the first holes (pyfatfs takes the first free clusters): fragmented around the fillers that stay.
    # The engine's guess skips the clusters in use, so it comes back whole - but it can't be called Good.
    write(fs, "/frag_live_gaps.bin", rnd.randbytes(37 * bpc + 123))
    expect("/frag_live_gaps.bin", recov="Partly overwritten")
    # plug the remaining holes with a live file, so the files below are contiguous
    region_end = max(used_clusters(fs))
    holes = [c for c in range(2, region_end) if not fs.fs.fat[c]]
    if holes:
        fs.writebytes("/PLUG.BIN", rnd.randbytes(len(holes) * bpc))

    # the lowest free clusters now - deleted first, then taken by /Keep/keeper.bin
    write(fs, "/overwritten.bin", rnd.randbytes(6 * bpc))
    expect("/overwritten.bin", recov="Overwritten", check_data=False)

    write(fs, "/contiguous.bin", rnd.randbytes(5 * bpc + 77))
    expect("/contiguous.bin")
    write(fs, "/empty.txt", b"")
    expect("/empty.txt")
    write(fs, "/Čačak ćevapi.txt", rnd.randbytes(3000))
    expect("/Čačak ćevapi.txt", content="Damaged")
    write(fs, "/фотографија.jpg", rnd.randbytes(3000))
    expect("/фотографија.jpg", content="Damaged")
    write(fs, "/Grüße.txt", rnd.randbytes(1500))
    expect("/Grüße.txt")
    # an 8.3 name without a long name: its first character is lost for good
    write(fs, "/README.TXT", b"Read me first.\r\n")
    expect("/README.TXT", name="_EADME.TXT", content="Looks valid")

    write(fs, "/preview.png", helper_output("make_png.py", 120, 80))
    expect("/preview.png", content="Looks valid")
    write(fs, "/letter.pdf", b"%PDF-1.4\n" + rnd.randbytes(20000))
    expect("/letter.pdf", content="Looks valid")

    # a file deleted from a folder that stays, and a folder deleted with its file
    fs.makedirs("/Photos/2014")
    write(fs, "/Photos/2014/holiday.png", helper_output("make_png.py", 64, 48))
    expect("/Photos/2014/holiday.png", folder="\\Photos\\2014", content="Looks valid")
    fs.makedir("/Old Stuff")
    write(fs, "/Old Stuff/notes.txt", b"Shopping list\r\n- milk\r\n- bread\r\nCall Ana about the trip.\r\n")
    expect("/Old Stuff/notes.txt", folder="\\Old Stuff", content="Looks valid")
    # a deleted folder bigger than one cluster: its later clusters lie after its files' data, its chain is gone
    fs.makedir("/Big Folder")
    for i in range(40):
        write(fs, f"/Big Folder/holiday photo {i:02d}.bin", rnd.randbytes(700 + i))
        expect(f"/Big Folder/holiday photo {i:02d}.bin", folder="\\Big Folder")

    # Recycle Bin: $R<6 chars><ext> holds the data, $I<same> the original path. $RQ1W2E3 has no long name -
    # in the Recycle Bin the lost first character can only be "$"
    fs.makedirs("/$RECYCLE.BIN/S-1-5-21-1")
    for r, i, version, original in (("$RA1B2C3.jpg", "$IA1B2C3.jpg", 1, "C:\\Users\\test\\Pictures\\Летовање 2014.jpg"),
                                    ("$RX9Y8Z7.docx", "$IX9Y8Z7.docx", 2, "C:\\Users\\test\\Documents\\Izveštaj.docx"),
                                    ("$RQ1W2E3", "$IQ1W2E3", 2, "C:\\Users\\test\\Desktop\\README")):
        folder = "/$RECYCLE.BIN/S-1-5-21-1/"
        write(fs, folder + r, rnd.randbytes(5000))
        write(fs, folder + i, helper_output("make_recycle_info.py", version, original, 5000))
        original_name = original.rsplit("\\", 1)[1]
        expect(folder + r, name=original_name, folder=original[2:original.rindex("\\")],
               content="Damaged" if original_name.endswith((".jpg", ".docx")) else "-")

    if fat_type == PyFat.FAT_TYPE_FAT32:
        # past cluster 65535: the first cluster needs the high 16 bits of the entry
        fs.writebytes("/BIGFILL.BIN", bytes((65600 - max(used_clusters(fs))) * bpc))
        write(fs, "/far.bin", rnd.randbytes(3 * bpc + 5))
        expect("/far.bin")
    fs.close()

    # delete overwritten.bin and let a new file take its clusters
    raw = RawFat(img)
    if fat_type == PyFat.FAT_TYPE_FAT32:
        assert raw.find("/far.bin")[3] > 0xFFFF, "far.bin should start past cluster 65535"
    raw.delete("/overwritten.bin")
    raw.save()
    fs = PyFatFS(img)
    fs.writebytes("/Keep/keeper.bin", rnd.randbytes(6 * bpc))
    fs.close()

    raw = RawFat(img)
    for path in files:
        if path != "/overwritten.bin" and not path.startswith(("/Old Stuff/", "/Big Folder/")):
            raw.delete(path)
    raw.delete("/Old Stuff")
    raw.delete("/Big Folder")
    raw.save()

    with open(os.path.join(out_dir, name + ".expected.tsv"), "w", encoding="utf-8", newline="\n") as f:
        for row in expected:
            f.write("\t".join(row) + "\n")
    print(f"{img}: FAT{raw.type}, {bpc}-byte clusters, {len(expected)} deleted test files")


def main():
    out_dir = sys.argv[1]
    os.makedirs(out_dir, exist_ok=True)
    build(out_dir, "fat12", PyFat.FAT_TYPE_FAT12, 2 * 1024 * 1024)
    build(out_dir, "fat16", PyFat.FAT_TYPE_FAT16, 16 * 1024 * 1024)
    build(out_dir, "fat32", PyFat.FAT_TYPE_FAT32, 40 * 1024 * 1024)


if __name__ == "__main__":
    main()
