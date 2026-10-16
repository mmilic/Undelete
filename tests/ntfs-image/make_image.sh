#!/bin/sh
# Builds out/ntfs-test/test.img: a small NTFS volume holding deleted files that are fragmented, sparse,
# resident (stored inside the MFT record) and long-named (attributes cross the 510-byte fixup point).
# expected.sha256 has the checksums of the files before they were deleted.
# Run as root in WSL (needs ntfs-3g): wsl -u root -e sh tests/ntfs-image/make_image.sh
set -e
D=$(cd "$(dirname "$0")/../.." && pwd)/out/ntfs-test
mkdir -p "$D"
IMG=$D/test.img
MNT=/tmp/undelete-ntfs-test
LONG=frag_long_name_$(printf 'x%.0s' $(seq 1 180)).bin

rm -f "$IMG" "$D/expected.sha256"
dd if=/dev/zero of="$IMG" bs=1M count=8 status=none
mkntfs -F -Q -q -c 4096 -L undeltest "$IMG"
mkdir -p "$MNT"
ntfs-3g "$IMG" "$MNT"

# fill the volume with 64 KB files, then free every other one -> lots of 64 KB holes
i=0
while head -c 65536 /dev/urandom > "$MNT/fill_$i.bin" 2>/dev/null; do i=$((i+1)); done
rm -f "$MNT/fill_$i.bin"
# top up the remaining free space (incl. the MFT zone) with 4 KB files so only the holes are left
k=0
while head -c 4096 /dev/urandom > "$MNT/small_$k.bin" 2>/dev/null; do k=$((k+1)); done
rm -f "$MNT/small_$k.bin"
# free every other 64 KB file *in disk order* (the allocator does not place them in creation order)
umount "$MNT"
j=0
while [ $j -lt $i ]; do
  lcn=$(ntfsinfo -v -F "/fill_$j.bin" "$IMG" | sed -n "/Runlist/{n;p}" | awk '{print $2}')
  echo "$((lcn)) fill_$j.bin"
  j=$((j+1))
done | sort -n | awk 'NR % 2 == 1 {print $2}' > /tmp/undelete-to-free.txt
ntfs-3g "$IMG" "$MNT"
while read f; do rm "$MNT/$f"; done < /tmp/undelete-to-free.txt
FREE=$(( (i/2) * 65536 ))

# the test files - sizes deliberately not cluster multiples
head -c $(( FREE*2/5 + 1234 )) /dev/urandom > "$MNT/fragmented.bin"
head -c $(( FREE/4 + 777 )) /dev/urandom > "$MNT/$LONG"
head -c 300 /dev/urandom > "$MNT/tiny_resident.bin"
head -c 6000 /dev/urandom > "$MNT/sparse.bin"
dd if=/dev/urandom of="$MNT/sparse.bin" bs=1 count=5000 seek=200000 conv=notrunc status=none

(cd "$MNT" && sha256sum fragmented.bin "$LONG" tiny_resident.bin sparse.bin) > "$D/expected.sha256"
umount "$MNT"

echo "--- data runs before deletion ---"
for f in fragmented.bin "$LONG" sparse.bin; do
  echo "$f" | cut -c1-40
  ntfsinfo -v -F "/$f" "$IMG" | sed -n "/Runlist/,/^$/p" | head -30
done

ntfs-3g "$IMG" "$MNT"
rm "$MNT/fragmented.bin" "$MNT/$LONG" "$MNT/tiny_resident.bin" "$MNT/sparse.bin"
umount "$MNT"
echo "image ready: $IMG"
