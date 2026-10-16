/* 
 * File:   NTFSDrive.h
 * Author: MilicM
 *
 * Created on September 28, 2012, 12:05 PM
 */

#ifndef NTFSDRIVE_H
#define	NTFSDRIVE_H



#endif	/* NTFSDRIVE_H */

// NTFSDrive.h: interface for the CNTFSDrive class.
//
//////////////////////////////////////////////////////////////////////

//#include <windows.h>
//#include <wx/wx.h>
#include <string>
#include <vector>
#include "MFTRecord.h"
//#include <wx/hashmap.h>

#if !defined(AFX_NTFSDRIVE_H__078B2392_2978_4C23_97FD_166C4B234BF3__INCLUDED_)
#define AFX_NTFSDRIVE_H__078B2392_2978_4C23_97FD_166C4B234BF3__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#pragma pack(push, curAlignment)

#pragma pack(1)
////////////////////////////  boot sector info ///////////////////////////////////////

struct NTFS_PART_BOOT_SEC {
    char chJumpInstruction[3];
    char chOemID[4];
    char chDummy[4];

    struct NTFS_BPB {
        WORD wBytesPerSec;
        BYTE uchSecPerClust;
        WORD wReservedSec;
        BYTE uchReserved[3];
        WORD wUnused1;
        BYTE uchMediaDescriptor;
        WORD wUnused2;
        WORD wSecPerTrack;
        WORD wNumberOfHeads;
        DWORD dwHiddenSec;
        DWORD dwUnused3;
        DWORD dwUnused4;
        LONGLONG n64TotalSec;
        LONGLONG n64MFTLogicalClustNum;
        LONGLONG n64MFTMirrLogicalClustNum;
        int nClustPerMFTRecord;
        int nClustPerIndexRecord;
        LONGLONG n64VolumeSerialNum;
        DWORD dwChecksum;
    } bpb;

    char chBootstrapCode[426];
    WORD wSecMark;
};
/////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop, curAlignment)

// one deleted file found by a scan. A big drive has hundreds of thousands of them, so keep this small:
// the name is sized to fit, and the status text is built from the flags only when a row is shown.
struct ST_FILEINFO // this struct is to retrieve the file detail from this class
{
    std::wstring sFilename; // file name - UTF-16, exactly as stored in the MFT
    LONGLONG n64Create; // Creation time
    LONGLONG n64Modify; // Last Modify time
    LONGLONG n64Modfil; // Last modify of record
    LONGLONG n64Access; // Last Access time
    DWORD dwAttributes; // file attribute
    bool bDeleted; // if true then its deleted file
    bool bInRecycleBin; // the name was recovered from a Recycle Bin $I file
    int iRecordId;
    LONGLONG n64SizeOnDisk;
    LONGLONG n64RealSize; // real size of the data, from the DATA attribute (FILE_NAME sizes are often stale)
    int iRecoverability; // RECOVERABILITY_*
    int iPercentIntact; // share of the file's clusters that are still free (not reused), 0..100

    ST_FILEINFO() : n64Create(0), n64Modify(0), n64Modfil(0), n64Access(0), dwAttributes(0), bDeleted(false),
    bInRecycleBin(false), iRecordId(0), n64SizeOnDisk(0), n64RealSize(0), iRecoverability(0), iPercentIntact(0) {
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

class CNTFSDrive {
protected:
    //////////////// physical drive info/////////////
    HANDLE m_hDrive;
    LONGLONG m_llStartSector;
    bool m_bInitialized;

    DWORD m_dwBytesPerCluster;
    DWORD m_dwBytesPerSector;

    int LoadMFT(LONGLONG nStartCluster);

    /////////////////// the MFT info /////////////////
    // The MFT is not loaded into memory as a whole - on a big volume it is gigabytes, more than this
    // 32-bit process can hold. Only its location is kept, and records are read through a small cache.
    std::vector<NTFS_DATA_RUN> m_vMFTRuns; // where the MFT is on the volume
    LONGLONG m_llMFTSize; // bytes of the MFT that can be read (multiple of the record size)
    std::vector<BYTE> m_vMFTCache; // a window of the MFT
    LONGLONG m_llMFTCacheStart; // MFT byte offset of m_vMFTCache[0]
    DWORD m_dwMFTCacheLen; // valid bytes in m_vMFTCache, 0 = empty

    int ReadRunsBytes(const std::vector<NTFS_DATA_RUN> &vRuns, LONGLONG llOffset, BYTE *puchOut, DWORD dwLen);
    int LoadMFTRecord(DWORD nFileSeq);

    // $Bitmap - one bit per cluster, set = in use. Loaded once per scan to judge recoverability.
    std::vector<BYTE> m_vVolumeBitmap;
    volatile bool *m_pbCancel;
    volatile LONG *m_plProgressKB;
    bool m_bOwnsHandle; // m_hDrive was opened by ReadVolume (not passed in with SetDriveHandle)
    bool IsCancelled() const { return m_pbCancel && *m_pbCancel; }
    int LoadVolumeBitmap();
    void CheckRecoverability(const CMFTRecord &cFile, t_FileInfo &stFileInfo);

    BYTE *m_puchMFTRecord; // 1K, or the cluster size, whichever is larger
    LONGLONG m_llMFTRecordSz; // MFT record size

public:


    int GetFileDetail(DWORD nFileSeq, ST_FILEINFO &stFileInfo);
    int Read_File(DWORD nFileSeq, BYTE *&puchFileData, DWORD &dwFileDataLen);

    void SetDriveHandle(HANDLE hDrive);
    void SetStartSector(DWORD dwStartSector, DWORD dwBytesPerSector);
    bool IsInitialised();
    int ReadVolume(wxString wxsSelectedDrive);
    int GetFileList(FileInfoHash *finfohash);

    // long operations (scan, recovery of one file) stop with ERROR_CANCELLED once *pbCancel becomes true
    void SetCancelFlag(volatile bool *pbCancel);

    // RetreiveAndWriteFileToDestination adds the KB it writes here (for the copy progress bar)
    void SetProgressCounter(volatile LONG *plKBWritten) { m_plProgressKB = plKBWritten; }
    void ResolveRecycleBinNames(FileInfoHash *finfohash);
    int RetreiveAndWriteFileToDestination(int iFileRecordId, const wchar_t *wszDestinationFileName);

    int Initialize();
    CNTFSDrive();
    virtual ~CNTFSDrive();

};

#endif // !defined(AFX_NTFSDRIVE_H__078B2392_2978_4C23_97FD_166C4B234BF3__INCLUDED_)

