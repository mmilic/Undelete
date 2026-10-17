#!/bin/sh
# Builds out/exfat-test/exfat.img: an exFAT volume, formatted by exfatprogs and written and deleted from by Linux's
# kernel exfat driver, holding deleted files - fragmented and contiguous, in a
# folder that stays and in deleted folders, a preallocated one, Recycle Bin pairs - and exfat.expected.tsv, what
# the engine must report for each deleted test file:
#   name, SHA-256 of the data, recoverability, folder, content check ("-": not checked), name it is saved under,
#   modification time (Unix seconds)
# Run as root (WSL or Linux, needs exfatprogs): wsl -u root -e sh tests/exfat-image/make_image.sh
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
D=$(cd "$HERE/../.." && pwd)/out/exfat-test
NTFS_TEST=$HERE/../ntfs-image # the PNG and $I file writers are shared with the NTFS test
IMG=$D/exfat.img
TSV=$D/exfat.expected.tsv
MNT=/tmp/undelete-exfat-test
mkdir -p "$D" "$MNT"
rm -f "$IMG" "$TSV"

# The kernel's exfat driver only. exfat-fuse deletes differently: it clears a deleted file's FAT chain and shrinks
# a folder before deleting it, so the expectations below (what Windows and the kernel driver leave) wouldn't hold.
if ! modprobe exfat; then
  echo "The kernel has no exfat module - on Ubuntu: apt-get install linux-modules-extra-\$(uname -r)" >&2
  exit 1
fi
mount_image() {
  mount -t exfat -o loop "$IMG" "$MNT"
}
unmount_image() {
  umount "$MNT"
}

dd if=/dev/zero of="$IMG" bs=1M count=16 status=none
mkfs.exfat -c 4096 -L UNDELTEST "$IMG" >/dev/null
mount_image

# fill the volume with 16 KB files, then free every other one -> 4-cluster holes between files in use
i=0
while head -c 16384 /dev/urandom > "$MNT/fill_$i.bin" 2>/dev/null; do i=$((i+1)); done
rm -f "$MNT/fill_$i.bin"
# Linux's exfat driver zeroes a deleted file's size and first cluster if the file is deleted while it is still
# cached from being written - delete only after a fresh mount, as with a file deleted later on
unmount_image
mount_image
j=0
while [ $j -lt $i ]; do rm "$MNT/fill_$j.bin"; j=$((j+2)); done
FREE=$(( (i / 2) * 16384 ))
echo "$i filler files, every other one freed: $FREE bytes in holes"

# expect <path under the volume> <recoverability> <folder> <content check or -> [<name>] [<saved as>]
expect() {
  f="$MNT/$1"
  name=${5:-$(basename "$1")}
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$name" "$(sha256sum < "$f" | cut -d' ' -f1)" "$2" "$3" "$4" "${6:-$name}" \
    "$(stat -c %Y "$f")" >> "$TSV"
}

# the test files - deleted last, so nothing written after them can take their clusters.
# Fragmented into the holes: exFAT keeps its chain in the FAT when it is deleted, so it is Good.
head -c $(( FREE * 2 / 5 + 1234 )) /dev/urandom > "$MNT/fragmented.bin"
head -c $(( 5 * 4096 + 77 )) /dev/urandom > "$MNT/contiguous.bin"
: > "$MNT/empty.txt"
head -c 3000 /dev/urandom > "$MNT/Čačak ćevapi.txt"
head -c 3000 /dev/urandom > "$MNT/фотографија.jpg"
head -c 1500 /dev/urandom > "$MNT/Grüße.txt"
python3 "$NTFS_TEST/make_png.py" 120 80 "$MNT/preview.png"
{ printf '%%PDF-1.4\n'; head -c 20000 /dev/urandom; } > "$MNT/letter.pdf"
# 200 characters - exFAT names go up to 255, with no 8.3 name beside them
LONG="long name $(printf 'x%.0s' $(seq 1 186)).bin"
head -c 900 /dev/urandom > "$MNT/$LONG"
# 5000 bytes written, then made 20000 long: the rest was never written, and reads as zeros
head -c 5000 /dev/urandom > "$MNT/preallocated.bin"
truncate -s 20000 "$MNT/preallocated.bin"

mkdir -p "$MNT/Photos/2014" "$MNT/Old Stuff" "$MNT/Names" "$MNT/Big Folder"
python3 "$NTFS_TEST/make_png.py" 64 48 "$MNT/Photos/2014/holiday.png"
printf 'Shopping list\r\n- milk\r\n- bread\r\nCall Ana about the trip.\r\n' > "$MNT/Old Stuff/notes.txt"
printf 'not the AUX device\r\n' > "$MNT/Names/aux.txt"
# a deleted folder bigger than one cluster: 40 entry sets of 4 entries (128 bytes) each
k=0
while [ $k -lt 40 ]; do
  head -c $(( 700 + k )) /dev/urandom > "$MNT/Big Folder/holiday photo $(printf %02d $k).bin"
  k=$((k+1))
done

# Recycle Bin: $R<6 chars><ext> holds the data, $I<same> the original path
RB="\$RECYCLE.BIN/S-1-5-21-1"
mkdir -p "$MNT/$RB"
recycled() { # <$R name> <$I name> <$I version> <original path>
  head -c 5000 /dev/urandom > "$MNT/$RB/$1"
  python3 "$NTFS_TEST/make_recycle_info.py" "$3" "$4" 5000 "$MNT/$RB/$2"
}
recycled '$RA1B2C3.jpg' '$IA1B2C3.jpg' 1 'C:\Users\test\Pictures\Летовање 2014.jpg'
recycled '$RX9Y8Z7.docx' '$IX9Y8Z7.docx' 2 'C:\Users\test\Documents\Izveštaj.docx'
recycled '$RQ1W2E3' '$IQ1W2E3' 2 'C:\Users\test\Desktop\README'

# a file that stays: it takes the rest of the freed filler clusters, so some deleted fillers really are
# overwritten - the engine must not call them "Good"
head -c 100000000 /dev/urandom > "$MNT/keeper.bin" 2>/dev/null || true
sync

for f in fragmented.bin contiguous.bin empty.txt "Grüße.txt" "$LONG" preallocated.bin; do expect "$f" Good '\' -; done
expect "Čačak ćevapi.txt" Good '\' Damaged
expect "фотографија.jpg" Good '\' Damaged
expect preview.png Good '\' "Looks valid"
expect letter.pdf Good '\' "Looks valid"
expect "Photos/2014/holiday.png" Good '\Photos\2014' "Looks valid"
expect "Old Stuff/notes.txt" Good '\Old Stuff' "Looks valid"
expect "Names/aux.txt" Good '\Names' - aux.txt _aux.txt
k=0
while [ $k -lt 40 ]; do expect "Big Folder/holiday photo $(printf %02d $k).bin" Good '\Big Folder' -; k=$((k+1)); done
expect "$RB/\$RA1B2C3.jpg" Good '\Users\test\Pictures' Damaged "Летовање 2014.jpg"
expect "$RB/\$RX9Y8Z7.docx" Good '\Users\test\Documents' Damaged "Izveštaj.docx"
expect "$RB/\$RQ1W2E3" Good '\Users\test\Desktop' - README

unmount_image
mount_image
cd "$MNT"
rm fragmented.bin contiguous.bin empty.txt "Čačak ćevapi.txt" "фотографија.jpg" "Grüße.txt" preview.png letter.pdf \
  "$LONG" preallocated.bin "Photos/2014/holiday.png" "Names/aux.txt" "$RB"/*
rm -r "Old Stuff" "Big Folder"
cd /
unmount_image
echo "image ready: $IMG ($(wc -l < "$TSV") deleted test files)"
