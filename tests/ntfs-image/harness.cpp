// Test harness: runs the real recovery engine (CNTFSDrive / CMFTRecord, or CFATDrive) against a disk image
// file instead of a live volume, and recovers every deleted file it finds into an output directory.
// The engine is chosen by the image's boot sector: NTFS if it is NTFS, FAT if it is FAT.
//   harness.exe <image> <output dir>
#include <windows.h>
#include <wx/wx.h>
#include <wx/init.h>
#include <wx/log.h>
#include <wx/image.h>
#include <wx/mstream.h>
#include <stdio.h>
#include "NTFSDrive.h"
#include "FileContents.h"
#if __has_include("FATDrive.h") // not in older engines (build_harness.ps1 -WithOld)
#define HAVE_FAT_ENGINE
#include "FATDrive.h"
#endif

// like the app's log file: UTF-8, so non-ANSI file names show up (wxLogStderr drops those lines)
class wxLogUtf8Stdout : public wxLog {
protected:
    virtual void DoLogText(const wxString& msg) {
        printf("%s\n", (const char *) msg.utf8_str());
    }
};

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: harness <image> <output dir>\n");
        return 2;
    }
    wxInitializer init;
    wxLog::SetActiveTarget(new wxLogUtf8Stdout);
    wxLog::SetTimestamp("");

    HANDLE hImage = CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (hImage == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "cannot open %s\n", argv[1]);
        return 1;
    }

    CNTFSDrive ntfs;
    ntfs.SetDriveHandle(hImage);
    ntfs.SetStartSector(0, 512);
    int nRet = ntfs.Initialize();
#ifdef HAVE_FAT_ENGINE
    CFATDrive fat;
    if (nRet == ERROR_INVALID_DRIVE) { // not NTFS
        fat.SetDriveHandle(hImage);
        nRet = fat.Initialize();
        if (!nRet)
            printf("FAT type: %d\n", fat.GetFATType());
    }
    CRecoveryDrive &drive = fat.GetFATType() ? (CRecoveryDrive &) fat : (CRecoveryDrive &) ntfs;
#else
    CNTFSDrive &drive = ntfs;
#endif
    if (nRet) {
        fprintf(stderr, "Initialize failed: %d\n", nRet);
        return 1;
    }

    FileInfoHash hash;
    drive.GetFileList(&hash);
    printf("deleted files found: %u\n", (unsigned) hash.size());

    // the copy progress bar's counter: KB written, over all files
    volatile LONG lKBWritten = 0;
    drive.SetProgressCounter(&lKBWritten);
    LONGLONG llBytesRecovered = 0;
    int iRecovered = 0;

    wxString outDir(argv[2]);
    for (FileInfoHash::iterator it = hash.begin(); it != hash.end(); ++it) {
        t_FileInfo *pInfo = it->second;
        wxString name(pInfo->sFilename.c_str());
#ifdef HAVE_FAT_ENGINE // the app's way to save a file: a name Windows accepts, long paths, the original dates
        wxString saved = SafeFileName(pInfo->sFilename);
#else
        wxString saved = name;
#endif
        wxString dest = outDir + "\\" + saved;
        drive.CheckFileContents(*pInfo); // what the "Check contents" button does
        nRet = drive.RetreiveAndWriteFileToDestination(pInfo->iRecordId, dest.wc_str());
        if (!nRet) {
#ifdef HAVE_FAT_ENGINE
            RestoreFileTimes(dest, *pInfo);
#endif
            llBytesRecovered += pInfo->n64RealSize;
            iRecovered++;
        }
        // tab separated, for run.ps1: record, name, recoverability, % intact, recovery result, folder, contents,
        // the name it was saved under, its modification time (FILETIME), the recovery's error code (0 = ok)
        printf("FILE\t%d\t%s\t%s\t%d\t%s\t%s\t%s\t%s\t%lld\t%d\n", pInfo->iRecordId, (const char *) name.utf8_str(),
                RecoverabilityLabel(pInfo->iRecoverability), pInfo->iPercentIntact, nRet ? "FAILED" : "ok",
                (const char *) wxString(pInfo->sFolder.c_str()).utf8_str(), ContentLabel(pInfo->iContent),
                (const char *) saved.utf8_str(), (long long) pInfo->n64Modify, nRet);
    }
    // each file can leave up to 1 KB uncounted (the counter is in whole KB)
    printf("PROGRESS\t%ld\t%ld\t%d\n", (long) lKBWritten, (long) (llBytesRecovered / 1024), iRecovered);

    // the image preview's path: read the deleted data into memory and decode it (like wxSelectFilesPage::OnPreviewTimer)
    wxInitAllImageHandlers();
    for (FileInfoHash::iterator it = hash.begin(); it != hash.end(); ++it) {
        t_FileInfo *pInfo = it->second;
        wxString name(pInfo->sFilename.c_str());
        if (!name.Lower().EndsWith(".png") && !name.Lower().EndsWith(".jpg"))
            continue;

        BYTE *puchData = NULL;
        DWORD dwLen = 0;
        wxImage image;
        if (!drive.Read_File(pInfo->iRecordId, puchData, dwLen)) {
            wxLogNull noLog;
            wxMemoryInputStream stream(puchData, dwLen);
            image.LoadFile(stream, wxBITMAP_TYPE_ANY);
            delete[] puchData;
        }
        if (image.IsOk())
            printf("PREVIEW\t%s\t%dx%d\n", (const char *) name.utf8_str(), image.GetWidth(), image.GetHeight());
        else
            printf("PREVIEW\t%s\tno image\n", (const char *) name.utf8_str());
    }
    return 0;
}
