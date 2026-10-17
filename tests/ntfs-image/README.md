# NTFS image test

End-to-end check of the recovery engine (`CNTFSDrive` / `CMFTRecord`) without admin rights or a real drive.

```powershell
powershell -ExecutionPolicy Bypass -File tests\ntfs-image\run.ps1
```

Needs WSL with `ntfs-3g` installed (`sudo apt-get install ntfs-3g`). Everything it produces goes to `out\ntfs-test\`.

## What it does

1. `make_image.sh` (runs as root in WSL) creates three NTFS images with the same test files, formatted
   differently:
   - `test.img`: 8 MB, 512-byte sectors and 4 KB clusters - the usual NTFS
   - `4k-sectors.img`: 4096-byte sectors, as on "4Kn" disks. The MFT records are 4 KB too, and the boot sector
     can't be read as 512 bytes.
   - `256k-clusters.img`: 256 KB clusters. Above 64 KB the boot sector stores the sectors per cluster as a
     power of two (`0xF7` = 2^9), not as a count.

   For each, it mounts the image with `ntfs-3g`, which can mount a plain image file directly, fills the
   volume, and frees every other filler file (64 KB, or two clusters) in disk order to leave holes. It then
   writes the test files, records their SHA-256 checksums in `<image>.sha256`, deletes them and unmounts.
   - `fragmented.bin`: 18 fragments on `test.img`
   - a file with a 199-character name, 11 fragments. The long name pushes the attributes past byte 510 of
     the MFT record, so the fixups have to be applied correctly.
   - `sparse.bin`: has a hole that isn't stored on disk
   - `tiny_resident.bin`: 300 bytes, stored inside the MFT record itself
   - `Čačak ćevapi.txt`, `фотографија.jpg`, `Grüße.txt`: names outside ASCII
   - three Recycle Bin pairs, `$R…` (the data) plus `$I…` (the original path), written by
     `make_recycle_info.py`. One uses the Vista–8.1 `$I` format with a Cyrillic path, one the Windows 10+ format,
     and one has no file extension. Each must come back under its original name.
   - `preview.png`, a real image written by `make_png.py`. The image preview's read-and-decode path must
     show it, and must reject `фотографија.jpg`, which is random bytes.
   - `Photos/2014/holiday.png` (the folder stays) and `Old Stuff/notes.txt` (the folder is deleted too): each
     must come back with its folder, `\Photos\2014` and `\Old Stuff`. The Recycle Bin files must get the
     folder from their original path, and their `$I` files must not be listed.
   - `letter.pdf`, 20 KB starting with `%PDF`: with the PNGs and the text file it must pass the content check
     ("Looks valid"), while the random-byte `.jpg`, `.txt` and `.docx` files must be "Damaged".
   - `keeper.bin`, which is not deleted: it takes the rest of the freed filler clusters, so some deleted
     fillers really are overwritten. The engine must flag those, and flag every test file Good.
2. `build_harness.ps1` builds the `engine_harness` CMake target (`harness.cpp` with the engine sources from
   `src\`, same toolchain as `build.ps1`). With `-WithOld` it also builds the engine and the harness from git
   `HEAD` in a second build tree, so you can compare before and after a change. The FAT test
   (`tests\fat-image`) uses the same harness - it picks the engine by the image's boot sector.
3. The harness opens each image as if it were a raw volume (`\\.\X:`), scans the MFT and recovers every deleted
   file. `run.ps1` then checks the test files against their original checksums and checks the recoverability
   each file was given. Its log for each image is `out
tfs-testun-<image>.log`.

## Limits

The image is created and the files are deleted by Linux's `ntfs-3g`, not by Windows. Windows-specific
behaviour still needs a manual test on a real (or virtual) drive formatted by Windows:
- what Windows leaves behind in a deleted file's MFT record
- real `$I`/`$R` files created by Windows (the test writes its own, following the documented format)
- compressed files
- files with an attribute list
