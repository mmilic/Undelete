/*
 * File:   RecoveryDrive.h
 *
 * What the wizard and its threads need from a recovery engine, whatever the file system: open a volume,
 * scan it for deleted files, read a deleted file's data and write it to the destination.
 * CNTFSDrive (NTFSDrive.h) and CFATDrive (FATDrive.h) implement it.
 */

#ifndef RECOVERYDRIVE_H
#define	RECOVERYDRIVE_H

#include <windows.h>
#include <wx/string.h>
#include <wx/hashmap.h>
#include <string>
#include <vector>

// one deleted file found by a scan. A big drive has hundreds of thousands of them, so keep this small:
// the name is sized to fit, and the status text is built from the flags only when a row is shown.
struct ST_FILEINFO // this struct is to retrieve the file detail from this class
{
    std::wstring sFilename; // file name - UTF-16, exactly as stored on the drive
    LONGLONG n64Create; // Creation time (UTC FILETIME)
    LONGLONG n64Modify; // Last Modify time
    LONGLONG n64Modfil; // Last modify of record
    LONGLONG n64Access; // Last Access time
    DWORD dwAttributes; // file attribute
    bool bDeleted; // if true then its deleted file
    bool bInRecycleBin; // the name was recovered from a Recycle Bin $I file
    int iRecordId; // the engine's id of the file: its MFT record (NTFS), its place in the scan's list (FAT)
    LONGLONG n64SizeOnDisk;
    LONGLONG n64RealSize; // real size of the data, from the DATA attribute (FILE_NAME sizes are often stale)
    int iRecoverability; // RECOVERABILITY_*
    int iPercentIntact; // share of the file's clusters that are still free (not reused), 0..100
    bool bStartReused; // its first cluster is in use by another file now (the start of the data is gone)
    int iContent; // CONTENT_* (FileContents.h) - checked on demand, not by the scan

    // the folder the file was in, e.g. "\Users\Ana\Pictures" ("\" = the root of the drive). A part that
    // can't be known any more (its folder record was reused) is "?". Files of one folder share the string.
    std::wstring sFolder;
    DWORD dwParentRecord; // NTFS: MFT record of the folder - with the sequence number it had when it was referenced
    WORD wParentSeq;
    WORD wSequence; // this record's own sequence number
    bool bDirectory;

    ST_FILEINFO() : n64Create(0), n64Modify(0), n64Modfil(0), n64Access(0), dwAttributes(0), bDeleted(false),
    bInRecycleBin(false), iRecordId(0), n64SizeOnDisk(0), n64RealSize(0), iRecoverability(0), iPercentIntact(0),
    bStartReused(false), iContent(0), dwParentRecord(0), wParentSeq(0), wSequence(0), bDirectory(false) {
    }
};
typedef ST_FILEINFO t_FileInfo;

// "File Status" column text, e.g. "Hidden, Deleted in Recycle Bin"
std::string FileStatusText(const t_FileInfo &stFileInfo);

// can the deleted file's data still be there? Judged by whether its clusters have been reused since -
// a free cluster may still hold the old data, a cluster in use by another file certainly doesn't.
enum {
    RECOVERABILITY_UNKNOWN = 0, // no cluster information (damaged/compressed record, bitmap not loaded)
    RECOVERABILITY_GOOD, // none of its clusters is in use - most likely intact
    RECOVERABILITY_PARTIAL, // some of its clusters are in use by other files
    RECOVERABILITY_OVERWRITTEN // all of its clusters are in use by other files
};

inline const char *RecoverabilityLabel(int iRecoverability) {
    switch (iRecoverability) {
        case RECOVERABILITY_GOOD: return "Good";
        case RECOVERABILITY_PARTIAL: return "Partly overwritten";
        case RECOVERABILITY_OVERWRITTEN: return "Overwritten";
        default: return "Unknown";
    }
}

//ovde je key tipa int - treba razmisliti da li je bolje da je key tipa string
WX_DECLARE_HASH_MAP(int, t_FileInfo*, wxIntegerHash, wxIntegerEqual, FileInfoHash);

// delete the files of a previous scan and empty the map (clear() alone leaks every t_FileInfo)
void DeleteFileInfoHash(FileInfoHash *finfohash);

class CRecoveryDrive {
public:
    CRecoveryDrive();
    virtual ~CRecoveryDrive();

    // "NTFS", "FAT32"... - for the log
    virtual const char *FileSystemName() const = 0;

    // open a drive letter ("E:" or "E:\") as a raw volume and load its file system structures.
    // Runs in the scan thread - no message boxes, the caller shows the error.
    // ERROR_INVALID_DRIVE: the volume doesn't hold this engine's file system.
    virtual int ReadVolume(wxString wxsSelectedDrive) = 0;

    // put every deleted file into the map. Returns ERROR_CANCELLED if the cancel flag was set -
    // the map then holds what was found so far.
    virtual int GetFileList(FileInfoHash *finfohash) = 0;

    // the whole file in a buffer allocated with new[] - the caller deletes it
    virtual int Read_File(DWORD dwFileId, BYTE *&puchFileData, DWORD &dwFileDataLen) = 0;

    // the first dwMax bytes of a file's data (fewer for a smaller file) - only the clusters needed are read
    virtual int ReadFileStart(DWORD dwFileId, std::vector<BYTE> &vData, DWORD dwMax) = 0;

    virtual int RetreiveAndWriteFileToDestination(int iFileId, const wchar_t *wszDestinationFileName) = 0;

    // check whether the start of the file still looks like its type, sets stFileInfo.iContent
    void CheckFileContents(t_FileInfo &stFileInfo);

    // long operations (scan, recovery of one file) stop with ERROR_CANCELLED once *pbCancel becomes true
    void SetCancelFlag(volatile bool *pbCancel) { m_pbCancel = pbCancel; }

    // RetreiveAndWriteFileToDestination adds the KB it writes here (for the copy progress bar)
    void SetProgressCounter(volatile LONG *plKBWritten) { m_plProgressKB = plKBWritten; }

protected:
    HANDLE m_hDrive;
    bool m_bOwnsHandle; // m_hDrive was opened by OpenVolume (not passed in with SetDriveHandle)
    volatile bool *m_pbCancel;
    volatile LONG *m_plProgressKB;

    bool IsCancelled() const { return m_pbCancel && *m_pbCancel; }

    // open "\\.\E:" for reading into m_hDrive (closing the previous scan's drive), returns the error
    int OpenVolume(const wxString &wxsSelectedDrive);
    void CloseVolume();

    // replace "$R...." names with the original file names stored in the matching "$I...." files
    void ResolveRecycleBinNames(FileInfoHash *finfohash);
};

#endif	/* RECOVERYDRIVE_H */
