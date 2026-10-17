# exFAT image test

End-to-end check of the exFAT recovery engine (`CExFATDrive`) without admin rights or a real card.

```powershell
powershell -ExecutionPolicy Bypass -File tests\exfat-image\run.ps1
```

Needs WSL with `exfatprogs` (`sudo apt-get install exfatprogs`) and `python3`. WSL's kernel mounts exFAT itself;
elsewhere `exfat-fuse` is used if the kernel has no exfat module. Everything it produces goes to `out\exfat-test\`.
With `-UseExistingImage` it skips making the image - the GitHub Actions build makes it on Linux and runs the
check on Windows.

## What it does

1. `make_image.sh` (runs as root in WSL) formats a 16 MB image with `mkfs.exfat` (4 KB clusters), mounts it and
   fills it with 16 KB files, then frees every other one to leave holes. It writes the test files, records what
   the engine must report for each in `exfat.expected.tsv`, then deletes them - with Linux's own exfat driver,
   so the image holds what a real deletion leaves behind - and unmounts.
   - `fragmented.bin`: spread over the holes. exFAT keeps a fragmented file's chain in the FAT when it is
     deleted, so it must come back whole and be called Good.
   - `contiguous.bin`, `empty.txt`, a 200-character name (exFAT keeps whole names, no 8.3 name beside them),
     `Čačak ćevapi.txt`, `фотографија.jpg`, `Grüße.txt`
   - `preallocated.bin`: 5000 bytes written, then extended to 20000 - the rest was never written and must come
     back as zeros (exFAT's ValidDataLength)
   - `preview.png` and `letter.pdf` (the content check must call them "Looks valid", the random-byte `.jpg` and
     `.txt` "Damaged")
   - `Photos/2014/holiday.png` (the folder stays), `Old Stuff/notes.txt` (the folder is deleted too) and
     `Big Folder` with 40 files - a deleted folder bigger than one cluster
   - `Names/aux.txt`: must be saved as `_aux.txt`, AUX being a device name
   - three Recycle Bin pairs (`$R…` + `$I…`): each must come back under its original name and folder, and the
     `$I` files must not be listed
   - `keeper.bin`, which is not deleted: it takes the rest of the freed filler clusters, so some deleted
     fillers really are overwritten and must not be called Good
2. `tests\ntfs-image\build_harness.ps1` builds the harness, which picks the engine by the image's boot sector.
3. The harness scans the image and recovers every deleted file. `run.ps1` checks each test file's data
   (SHA-256), recoverability, folder, content check, the name it is saved under, its modification time against
   the one Linux wrote, and the recovered file's date. Its log is `out\exfat-test\run-exfat.log`.

## Limits

The image is written by Linux, not by Windows. `tests\windows-image` (CI) also makes an exFAT volume with
Windows itself and checks the recovery from it.
