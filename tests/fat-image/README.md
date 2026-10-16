# FAT image test

End-to-end check of the FAT recovery engine (`CFATDrive`) on FAT12, FAT16 and FAT32, without admin rights or
a real drive.

```powershell
powershell -ExecutionPolicy Bypass -File tests\fat-image\run.ps1
```

Needs Python 3 on Windows. The first run makes a venv in `out\fat-test\venv` and installs `pyfatfs` into it
(pinned in `run.ps1`), so it needs internet access once. No WSL. Everything it produces goes to `out\fat-test\`.

## What it does

1. `make_image.py` makes `fat12.img` (2 MB, 1 KB clusters), `fat16.img` (16 MB, 2 KB clusters) and `fat32.img`
   (40 MB, 512-byte clusters). [pyfatfs](https://github.com/nathanhi/pyfatfs) formats and fills them - a FAT
   implementation independent of the engine's.
   pyfatfs removes a file by rewriting its whole folder, which leaves no deleted entry behind. So the test
   files are deleted by `make_image.py` itself, the way Windows deletes: the first byte of the 8.3 entry and of
   its long name entries becomes `0xE5`, the cluster chain is zeroed in every FAT, and a folder's files go
   before the folder. pyfatfs is still used for its own clean-up of the filler files, which leave no trace.
   - Filler files are written, then every other one is removed. `frag_live_gaps.bin` fills the holes, fragmented
     around the fillers that stay. The engine must recover it byte-for-byte and flag it "Partly overwritten".
     A partly overwritten file would look the same, so it can't be called Good.
   - `overwritten.bin` is deleted, then `Keep\keeper.bin` is written and takes its clusters. It must be
     flagged Overwritten.
   - `contiguous.bin`, `empty.txt` (0 bytes), names outside ASCII (`Čačak ćevapi.txt`, `фотографија.jpg`,
     `Grüße.txt`): Good, recovered byte-for-byte. Their long names must come back whole.
   - `README.TXT` has only an 8.3 name. Its first character is lost, so it must come back as `_EADME.TXT`.
   - `preview.png` must decode in the preview, `фотографија.jpg` (random bytes) must not. `preview.png`,
     `letter.pdf` and the text files must pass the content check, and the random-byte `.jpg`, `.txt` and
     `.docx` files must be "Damaged".
   - `Photos\2014\holiday.png` (the folder stays) and `Old Stuff\notes.txt` (the folder is deleted too) must come
     back with their folders. The deleted folder is read from its first cluster, which is free.
   - `Big Folder` with 40 long-named files is deleted too. It needs several clusters, with its files' data
     between them, so its chain has to be guessed. All 40 files must be found, in `\Big Folder`.
   - Three Recycle Bin pairs in `$RECYCLE.BIN\S-1-5-21-1`, written by the NTFS test's
     `make_recycle_info.py`. Each `$R` file must come back under its original name and folder, and the `$I`
     files must not be listed. `$RQ1W2E3` has no long name: its lost first character can only be `$`.
   - FAT32 only: `far.bin` starts past cluster 65535, so its first cluster needs the high 16 bits of the entry.

   Each image gets `<image>.expected.tsv`: the name, the SHA-256, the recoverability, the folder and the content
   check result each deleted test file must get.
2. The harness is the NTFS test's: `tests\ntfs-image\build_harness.ps1` builds it, and it picks the engine
   by the image's boot sector.
3. The harness recovers every deleted file of each image into `out\fat-test\recovered-<image>`. `run.ps1`
   checks the results against the expected list (the log is `out\fat-test\run-<image>.log`).

`-UseExistingImages` skips step 1.

## Limits

- Windows-specific behaviour still needs a manual test on a USB stick or memory card formatted and used by
  Windows:
  - what Windows really leaves in a deleted entry;
  - whether it clears the high 16 bits of the first cluster on FAT32 (the test assumes it doesn't);
  - real `$I`/`$R` files.
- A file fragmented around space that is free now comes back wrong, and is flagged Good - see "FAT support"
  in the main README. Nothing on the volume shows it, so there is no test for it.
- exFAT is not supported yet.
