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

typedef struct ST_FILEINFO // this struct is to retrieve the file detail from this class
{
    char szFilename[_MAX_PATH]; // file name
    LONGLONG n64Create; // Creation time
    LONGLONG n64Modify; // Last Modify time
    LONGLONG n64Modfil; // Last modify of record
    LONGLONG n64Access; // Last Access time
    DWORD dwAttributes; // file attribute
    LONGLONG n64Size; // no of cluseters used
    bool bDeleted; // if true then its deleted file
    char szBuffer[255];
    int iRecordId;
    LONGLONG n64SizeOnDisk;
} t_FileInfo;

//ovde je key tipa int - treba razmisliti da li je bolje da je key tipa string
WX_DECLARE_HASH_MAP(int, t_FileInfo*, wxIntegerHash, wxIntegerEqual, FileInfoHash);

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
    BYTE *m_puchMFT; /// the var to hold the loaded entire MFT
    DWORD m_dwMFTLen; // size of MFT

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
    int RetreiveAndWriteFileToDestination(int iFileRecordId, char *fnDestintationFileName);

    int Initialize();
    CNTFSDrive();
    virtual ~CNTFSDrive();

};

#endif // !defined(AFX_NTFSDRIVE_H__078B2392_2978_4C23_97FD_166C4B234BF3__INCLUDED_)

