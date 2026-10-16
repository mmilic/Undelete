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
#include "RecoveryDrive.h"
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

class CNTFSDrive : public CRecoveryDrive {
protected:
    //////////////// physical drive info/////////////
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
    int LoadVolumeBitmap();
    void CheckRecoverability(const CMFTRecord &cFile, t_FileInfo &stFileInfo);

public:
    // folders seen by the scan (in use and deleted), in record order - to rebuild each file's folder path
    struct ST_FOLDER {
        DWORD dwRecord;
        DWORD dwParentRecord;
        WORD wParentSeq;
        WORD wSequence;
        bool bInUse;
        std::wstring sName;
    };
protected:
    void ResolveFolders(FileInfoHash *finfohash, const std::vector<ST_FOLDER> &vFolders);
    std::vector<ST_FOLDER> m_vFolders; // what ScanFiles saw, for FinishScan

    BYTE *m_puchMFTRecord; // 1K, or the cluster size, whichever is larger
    LONGLONG m_llMFTRecordSz; // MFT record size

public:


    const char *FileSystemName() const override { return "NTFS"; }

    int GetFileDetail(DWORD nFileSeq, ST_FILEINFO &stFileInfo);
    int Read_File(DWORD nFileSeq, BYTE *&puchFileData, DWORD &dwFileDataLen) override;
    int ReadFileStart(DWORD nFileSeq, std::vector<BYTE> &vData, DWORD dwMax) override;

    void SetDriveHandle(HANDLE hDrive);
    void SetStartSector(DWORD dwStartSector, DWORD dwBytesPerSector);
    bool IsInitialised();
    int ReadVolume(wxString wxsSelectedDrive) override;
    int ScanFiles(FileInfoHash *finfohash) override;
    void FinishScan(FileInfoHash *finfohash) override;
    int RetreiveAndWriteFileToDestination(int iFileRecordId, const wchar_t *wszDestinationFileName) override;

    int Initialize();
    CNTFSDrive();
    virtual ~CNTFSDrive();

};

#endif // !defined(AFX_NTFSDRIVE_H__078B2392_2978_4C23_97FD_166C4B234BF3__INCLUDED_)

