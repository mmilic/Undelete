# Windows-written image test

End-to-end check of the recovery engine on a volume that **Windows itself** formatted and wrote. The NTFS and
FAT tests make their images with ntfs-3g and pyfatfs; this one catches what only Windows does.

```powershell
powershell -ExecutionPolicy Bypass -File tests\windows-image\run.ps1
```

It needs **administrator rights**, because it attaches virtual disks, so run it from an elevated PowerShell.
The GitHub build runs it on every push: GitHub's Windows machines run as admin. Everything it produces goes to
`out\windows-test\`. With `-UseExistingImages` it skips making the images and only checks them, which needs
no admin rights.

## What it does

1. For each image, it creates a 64 MB fixed VHD with `diskpart` (no Hyper-V needed), attaches it, and has
   Windows partition and format it (`Format-Volume`). Then it:
   - fills the volume with 64 KB files and frees every other one, so the test files are fragmented
   - writes the test files: `fragmented.bin`, names outside ASCII, a real PNG, a 150-character name,
     `README.TXT`, and files in folders (`Photos\2014` stays, `Old Stuff` is deleted with its file)
   - on NTFS also writes a compressed file (`compact /c`) and an EFS-encrypted one (`cipher /e`). The engine
     must refuse both, with `ERROR_NOT_SUPPORTED` and `ERROR_FILE_ENCRYPTED`, and leave no file behind.
   - deletes a file to the **real Recycle Bin**, through the shell with all dialogs off, then empties it. The
     engine must give the `$R` file its original name and folder back from Windows' own `$I` file.
   - flushes the volume (`Write-VolumeCache`), then deletes the test files. Without the flush, a file deleted
     right after it was written never had its data on the disk.
   - detaches the VHD and copies the volume out of it into `<image>.img`. The harness reads a volume, not a
     partitioned disk.

   What each file should give is written to `<image>.expected.tsv`: its checksum, recoverability, folder,
   the recovery's error code, and the name it is saved under.
2. It builds the harness (`tests\ntfs-image\build_harness.ps1`), the same one the NTFS and FAT tests use.
3. It recovers every deleted file from each image and checks the results against `<image>.expected.tsv`. The
   log for each image is `run-<image>.log`.

## Images

- `ntfs`: NTFS with 4 KB clusters.
- **FAT32 is not tested yet**: the engine fails on it. When Windows deletes a file on FAT32, it clears the
  high word of the file's first cluster number. On a volume with more than 65,535 clusters, which is every
  real USB stick and memory card, a file past that cluster then points to the wrong clusters. The FAT test
  doesn't show this: its images are small, and pyfatfs keeps the high word. The FAT32 image goes back into
  `$Images` in `run.ps1` once the engine finds the right first cluster again. On FAT the Recycle Bin part is
  skipped: the shell hangs there on the CI machine, and USB sticks and memory cards have no Recycle Bin.
