#!/bin/sh
# Builds the NTFS test images in out/ntfs-test: small NTFS volumes holding deleted files that are fragmented,
# sparse, resident (stored inside the MFT record) and long-named (attributes cross the 510-byte fixup point).
# <name>.sha256 has the checksums of the files before they were deleted. The same files go on each image,
# formatted differently:
#   test          512-byte sectors, 4 KB clusters - the usual NTFS
#   4k-sectors    4096-byte sectors ("4Kn" disks): 4 KB MFT records, the boot sector can't be read as 512 bytes
#   256k-clusters 256 KB clusters: the boot sector stores the sectors per cluster as a power of two (0xF7)
# Run as root in WSL (needs ntfs-3g): wsl -u root -e sh tests/ntfs-image/make_image.sh
set -e
D=$(cd "$(dirname "$0")/../.." && pwd)/out/ntfs-test
HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$D"
MNT=/tmp/undelete-ntfs-test
LONG=frag_long_name_$(printf 'x%.0s' $(seq 1 180)).bin

make_image() { # <name> <bytes per sector> <bytes per cluster>
  NAME=$1
  CLUSTER=$3
  IMG=$D/$NAME.img
  SUM=$D/$NAME.sha256
  # the filler files: 64 KB, at least two clusters - freeing every other one leaves the holes the test files
  # are fragmented into
  FILL=65536
  [ $FILL -lt $((CLUSTER * 2)) ] && FILL=$((CLUSTER * 2))
  # 8 MB, or room for ~150 fillers when the clusters are big
  MB=8
  [ $MB -lt $((FILL * 150 / 1048576)) ] && MB=$((FILL * 150 / 1048576))
  echo "=== $NAME: $2-byte sectors, $CLUSTER-byte clusters, $MB MB ==="

  rm -f "$IMG" "$SUM"
  dd if=/dev/zero of="$IMG" bs=1M count=$MB status=none
  mkntfs -F -Q -q -s "$2" -c "$CLUSTER" -L undeltest "$IMG"
  mkdir -p "$MNT"
  ntfs-3g "$IMG" "$MNT"

  # fill the volume with filler files, then free every other one -> lots of holes
  i=0
  while head -c $FILL /dev/urandom > "$MNT/fill_$i.bin" 2>/dev/null; do i=$((i+1)); done
  rm -f "$MNT/fill_$i.bin"
  # top up the remaining free space (incl. the MFT zone) with one-cluster files so only the holes are left
  k=0
  while head -c "$CLUSTER" /dev/urandom > "$MNT/small_$k.bin" 2>/dev/null; do k=$((k+1)); done
  rm -f "$MNT/small_$k.bin"
  # free every other filler *in disk order* (the allocator does not place them in creation order)
  umount "$MNT"
  j=0
  while [ $j -lt $i ]; do
    lcn=$(ntfsinfo -v -F "/fill_$j.bin" "$IMG" | sed -n "/Runlist/{n;p}" | awk '{print $2}')
    echo "$((lcn)) fill_$j.bin"
    j=$((j+1))
  done | sort -n | awk 'NR % 2 == 1 {print $2}' > /tmp/undelete-to-free.txt
  ntfs-3g "$IMG" "$MNT"
  while read f; do rm "$MNT/$f"; done < /tmp/undelete-to-free.txt
  FREE=$(( (i/2) * FILL ))

  # the test files - sizes deliberately not cluster multiples
  head -c $(( FREE*2/5 + 1234 )) /dev/urandom > "$MNT/fragmented.bin"
  head -c $(( FREE/4 + 777 )) /dev/urandom > "$MNT/$LONG"
  head -c 300 /dev/urandom > "$MNT/tiny_resident.bin"
  head -c 6000 /dev/urandom > "$MNT/sparse.bin"
  dd if=/dev/urandom of="$MNT/sparse.bin" bs=1 count=5000 seek=$((CLUSTER * 50)) conv=notrunc status=none

  # names outside ASCII - Serbian Latin/Cyrillic and German
  head -c 3000 /dev/urandom > "$MNT/Čačak ćevapi.txt"
  head -c 3000 /dev/urandom > "$MNT/фотографија.jpg"
  head -c 3000 /dev/urandom > "$MNT/Grüße.txt"

  # a real image (the preview must decode it) - "фотографија.jpg" above is random bytes (the preview must not)
  python3 "$HERE/make_png.py" 120 80 "$MNT/preview.png"

  # folders: a file deleted from a folder that stays, a folder deleted with its file, and a PDF whose start
  # (not resident - 20 KB) must pass the content check
  mkdir -p "$MNT/Photos/2014" "$MNT/Old Stuff"
  python3 "$HERE/make_png.py" 64 48 "$MNT/Photos/2014/holiday.png"
  printf 'Shopping list\r\n- milk\r\n- bread\r\nCall Ana about the trip.\r\n' > "$MNT/Old Stuff/notes.txt"
  { printf '%%PDF-1.4\n'; head -c 20000 /dev/urandom; } > "$MNT/letter.pdf"

  (cd "$MNT" && sha256sum fragmented.bin "$LONG" tiny_resident.bin sparse.bin \
    "Čačak ćevapi.txt" "фотографија.jpg" "Grüße.txt" preview.png letter.pdf) > "$SUM"
  (cd "$MNT/Photos/2014" && sha256sum holiday.png) >> "$SUM"
  (cd "$MNT/Old Stuff" && sha256sum notes.txt) >> "$SUM"

  # Recycle Bin: the deleted file is $R<6 chars><ext>, its original path is in the matching $I file.
  # The engine should recover each $R file under its original name.
  recycled() { # <$R name> <$I name> <$I version> <original path>
    head -c 5000 /dev/urandom > "$MNT/$1"
    python3 "$HERE/make_recycle_info.py" "$3" "$4" 5000 "$MNT/$2"
    echo "$(sha256sum < "$MNT/$1" | cut -d' ' -f1)  ${4##*\\}" >> "$SUM"
  }
  recycled '$RA1B2C3.jpg' '$IA1B2C3.jpg' 1 'C:\Users\test\Pictures\Летовање 2014.jpg'
  recycled '$RX9Y8Z7.docx' '$IX9Y8Z7.docx' 2 'C:\Users\test\Documents\Izveštaj.docx'
  recycled '$RQ1W2E3' '$IQ1W2E3' 2 'C:\Users\test\Desktop\README'

  # a file that stays: it takes the rest of the freed filler clusters, so those deleted fillers are really
  # overwritten (clusters in use) - the engine must not call them "Good"
  head -c 1000000000 /dev/urandom > "$MNT/keeper.bin" 2>/dev/null || true
  umount "$MNT"

  echo "--- data runs before deletion ---"
  for f in fragmented.bin "$LONG" sparse.bin; do
    echo "$f" | cut -c1-40
    ntfsinfo -v -F "/$f" "$IMG" | sed -n "/Runlist/,/^$/p" | head -30
  done

  ntfs-3g "$IMG" "$MNT"
  rm "$MNT/fragmented.bin" "$MNT/$LONG" "$MNT/tiny_resident.bin" "$MNT/sparse.bin" \
    "$MNT/Čačak ćevapi.txt" "$MNT/фотографија.jpg" "$MNT/Grüße.txt" "$MNT/preview.png" "$MNT/letter.pdf" \
    "$MNT/Photos/2014/holiday.png" "$MNT"/'$R'* "$MNT"/'$I'*
  rm -r "$MNT/Old Stuff"
  umount "$MNT"
  echo "image ready: $IMG"
}

make_image test 512 4096
make_image 4k-sectors 4096 4096
make_image 256k-clusters 512 262144
