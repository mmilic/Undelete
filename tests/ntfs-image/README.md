# NTFS image test

End-to-end check of the recovery engine (`CNTFSDrive` / `CMFTRecord`) without admin rights or a real drive.

```powershell
powershell -ExecutionPolicy Bypass -File tests\ntfs-image\run.ps1
```

Needs WSL with `ntfs-3g` installed (`sudo apt-get install ntfs-3g`). Everything it produces goes to `out\ntfs-test\`.

## What it does

1. `make_image.sh` (runs as root in WSL) creates an 8 MB NTFS image. It mounts the image with `ntfs-3g`, which
   can mount a plain image file directly, fills the volume, and frees every other 64 KB file in disk order to
   leave holes. It then writes the test files, records their SHA-256 checksums, deletes them and unmounts.
   - `fragmented.bin`: 18 fragments
   - a file with a 199-character name, 11 fragments. The long name pushes the attributes past byte 510 of
     the MFT record, so the fixups have to be applied correctly.
   - `sparse.bin`: has a hole that isn't stored on disk
   - `tiny_resident.bin`: 300 bytes, stored inside the MFT record itself
2. `build_harness.ps1` compiles `harness.cpp` with the engine sources from `src\`. With `-WithOld` it also
   builds the engine from git `HEAD`, so you can compare before and after a change.
3. The harness opens `test.img` as if it were a raw volume (`\\.\X:`), scans the MFT and recovers every deleted
   file. `run.ps1` then checks the test files against their original checksums.

## Limits

The image is created and the files are deleted by Linux's `ntfs-3g`, not by Windows. Windows-specific
behaviour still needs a manual test on a real (or virtual) drive formatted by Windows:
- what Windows leaves behind in a deleted file's MFT record
- `$I`/`$R` names in the Recycle Bin
- compressed files
- files with an attribute list
