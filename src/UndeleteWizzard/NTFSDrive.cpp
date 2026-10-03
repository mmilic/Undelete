
// NTFSDrive.cpp: implementation of the CNTFSDrive class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include <windows.h>
#include <wx/wx.h>
#include <wx/hashmap.h>
#include "NTFSDrive.h"
#include "MFTRecord.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNTFSDrive::CNTFSDrive() {
    m_bInitialized = false;

    m_hDrive = 0;
    m_dwBytesPerCluster = 0;
    m_dwBytesPerSector = 0;

    m_puchMFTRecord = 0;
    m_llMFTRecordSz = 0;

    m_puchMFT = 0;
    m_dwMFTLen = 0;

    m_llStartSector = 0;
}

bool CNTFSDrive::IsInitialised() {
    return this->m_bInitialized;
}

CNTFSDrive::~CNTFSDrive() {
    if (m_puchMFT)
        delete m_puchMFT;
    m_puchMFT = 0;
    m_dwMFTLen = 0;
}

void CNTFSDrive::SetDriveHandle(HANDLE hDrive) {
    m_hDrive = hDrive;
    m_bInitialized = false;
}

// this is necessary to start reading a logical drive 

void CNTFSDrive::SetStartSector(DWORD dwStartSector, DWORD dwBytesPerSector) {
    m_llStartSector = dwStartSector;
    m_dwBytesPerSector = dwBytesPerSector;
}

// initialize will read the MFT record
///   and passes to the LoadMFT to load the entire MFT in to the memory

int CNTFSDrive::Initialize() {
    NTFS_PART_BOOT_SEC ntfsBS;
    DWORD dwBytes;
    LARGE_INTEGER n84StartPos;
    DWORD dwCur;

    n84StartPos.QuadPart = (LONGLONG) m_dwBytesPerSector*m_llStartSector;

    // point the starting NTFS volume sector in the physical drive
    dwCur = SetFilePointer(m_hDrive, n84StartPos.LowPart, &n84StartPos.HighPart, FILE_BEGIN);
    if (INVALID_SET_FILE_POINTER == dwCur) {
        wxLogFatalError("Unable to Set File Pointer - check disk access rights");
        return GetLastError(); // heheh - dovde nece doci nikad :) wxLogFatalError prekida izvrsavanje programa
    }

    // Read the boot sector for the MFT infomation
    int nRet = ReadFile(m_hDrive, &ntfsBS, sizeof (NTFS_PART_BOOT_SEC), &dwBytes, NULL);
    if (!nRet) {
        DWORD dwErrorCode = GetLastError();
        wxLogError("Unable to read from drive - check disk access rights!!");
        wxLogError("ReadRaw returned error code %d .", dwErrorCode); //sranje - ovde treba provera mesta na hardu i neko lepo preskakanje
        return (dwErrorCode);
    }

    if (memcmp(ntfsBS.chOemID, "NTFS", 4)) // check whether it is realy ntfs
        return (ERROR_INVALID_DRIVE);

    /// Cluster is the logical entity
    ///  which is made up of several sectors (a physical entity) 
    m_dwBytesPerCluster = ntfsBS.bpb.uchSecPerClust * ntfsBS.bpb.wBytesPerSec;

    if (m_puchMFTRecord)
        delete m_puchMFTRecord;

    m_llMFTRecordSz = 0x01 << ((-1)*((char) ntfsBS.bpb.nClustPerMFTRecord));
    m_puchMFTRecord = new BYTE[m_llMFTRecordSz];

    m_bInitialized = true;

    // MFTRecord of MFT is available in the MFTRecord variable
    //   load the entire MFT using it
    nRet = LoadMFT(ntfsBS.bpb.n64MFTLogicalClustNum);
    if (nRet) {
        m_bInitialized = false;
        return nRet;
    }
    return (ERROR_SUCCESS);
}

//// nStartCluster is the MFT table starting cluster
///    the first entry of record in MFT table will always have the MFT record of itself

int CNTFSDrive::LoadMFT(LONGLONG nStartCluster) {
    DWORD dwBytes;
    int nRet;
    DWORD dwRet;
    LARGE_INTEGER n64Pos;

    if (!m_bInitialized)
        return ERROR_INVALID_ACCESS;

    CMFTRecord cMFTRec;

    wchar_t uszMFTName[10];
    mbstowcs(uszMFTName, "$MFT", 10);

    // NTFS starting point
    n64Pos.QuadPart = (LONGLONG) m_dwBytesPerSector*m_llStartSector;
    // MFT starting point
    n64Pos.QuadPart += (LONGLONG) nStartCluster*m_dwBytesPerCluster;

    //  set the pointer to the MFT start
    dwRet = SetFilePointer(m_hDrive, n64Pos.LowPart, &n64Pos.HighPart, FILE_BEGIN);
    if (dwRet == INVALID_SET_FILE_POINTER) {
        wxLogFatalError("Unable to Set File Pointer - check disk access rights");
        return GetLastError(); // heheh - dovde nece doci nikad :) wxLogFatalError prekida izvrsavanje programa
    }

    /// reading the first record in the NTFS table.
    //   the first record in the NTFS is always MFT record
    nRet = ReadFile(m_hDrive, m_puchMFTRecord, m_llMFTRecordSz, &dwBytes, NULL);
    if (!nRet) {
        DWORD dwErrorCode = GetLastError();
        wxLogError("ReadRaw returned error code %d .", dwErrorCode); //sranje - ovde treba provera mesta na hardu i neko lepo preskakanje
        return (dwErrorCode);
    }

    // now extract the MFT record just like the other MFT table records
    cMFTRec.SetDriveHandle(m_hDrive);
    cMFTRec.SetRecordInfo((LONGLONG) m_llStartSector*m_dwBytesPerSector, m_llMFTRecordSz, m_dwBytesPerCluster);
    nRet = cMFTRec.ExtractFile(m_puchMFTRecord, dwBytes);
    if (nRet)
        return nRet;

    if (memcmp(cMFTRec.m_attrFilename.wFilename, uszMFTName, 8))
        return ERROR_BAD_DEVICE; // no MFT file available

    if (m_puchMFT)
        delete m_puchMFT;
    m_puchMFT = 0;
    m_dwMFTLen = 0;

    // this data(m_puchFileData) is special since it is the data of entire MFT file
    m_puchMFT = new BYTE[cMFTRec.m_llFileDataSz];
    m_dwMFTLen = cMFTRec.m_llFileDataSz;

    // store this file to read other files
    memcpy(m_puchMFT, cMFTRec.m_puchFileData, m_dwMFTLen);

    return ERROR_SUCCESS;
}


/// this function if succeeded it will allocate the buffer and passed to the caller
//    the caller's responsibility to free it

int CNTFSDrive::Read_File(DWORD nFileSeq, BYTE *&puchFileData, DWORD &dwFileDataLen) {
    int nRet;

    if (!m_bInitialized) {
        wxLogError("Calling Read_File for a drive that is not initialized.");
        return ERROR_INVALID_ACCESS;
    }

    CMFTRecord cFile;

    // point the record of the file in the MFT table
    memcpy(m_puchMFTRecord, &m_puchMFT[nFileSeq * m_llMFTRecordSz], m_llMFTRecordSz);

    // Then extract that file from the drive
    cFile.SetDriveHandle(m_hDrive);
    cFile.SetRecordInfo((LONGLONG) m_llStartSector*m_dwBytesPerSector, m_llMFTRecordSz, m_dwBytesPerCluster);
    nRet = cFile.ExtractFile(m_puchMFTRecord, m_llMFTRecordSz);
    if (nRet) {
        wxLogMessage("ExtractFile failed in Read_File");
        return nRet;
    }

    puchFileData = new BYTE[cFile.m_llFileDataSz];
    dwFileDataLen = cFile.m_llFileDataSz;

    // pass the file data, It should be deallocated by the caller
    memcpy(puchFileData, cFile.m_puchFileData, dwFileDataLen);

    return ERROR_SUCCESS;
}

int CNTFSDrive::ReadVolume(wxString wxsSelectedDir) {

    NTFS_PART_BOOT_SEC ntfsBS;
    DWORD dwBytes;
    LARGE_INTEGER n84StartPos;

    char tempDrive[10];
    memset(tempDrive, '\0', 10 * sizeof (char));
    strncpy(tempDrive, "\\\\.\\", 7);
    strncat(tempDrive, wxsSelectedDir.mb_str(), 2);

    wchar_t rDrive[10];
    memset(rDrive, '\0', 10 * sizeof (wchar_t));

    mbstowcs(rDrive, tempDrive, 9);


    memset(&n84StartPos, '\0', sizeof (n84StartPos));
    memset(&ntfsBS, '\0', sizeof (ntfsBS));
    DWORD lpSectorsPerCluster, lpBytesPerSector, lpNumberOfFreeClusters, lpTotalNumberOfClusters;

    m_hDrive = CreateFile(rDrive, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (m_hDrive == INVALID_HANDLE_VALUE) {
        wxMessageDialog *dial = new wxMessageDialog(NULL,
                wxT("Error loading file\nCheck Access rights!"), wxT("Error"), wxOK | wxICON_ERROR);
        dial->ShowModal();
        return (-2);
    }


    // ovaj korak je ovde malo cudan - al ajd - nek bude tu dok ne vidim kako cemo
    //    this->SetDriveHandle(hDrive);

    //    GetDiskFreeSpace(wxsSelectedDir.mb_str(), &lpSectorsPerCluster, &lpBytesPerSector, &lpNumberOfFreeClusters, &lpTotalNumberOfClusters);
    GetDiskFreeSpace(rDrive, &lpSectorsPerCluster, &lpBytesPerSector, &lpNumberOfFreeClusters, &lpTotalNumberOfClusters);

    m_dwBytesPerCluster = lpSectorsPerCluster*lpBytesPerSector;
    m_dwBytesPerSector = lpBytesPerSector;

    // opet nezgodno reseno, m_hDrive je postavljen funkcijom SetDriveHanle par redova iznad
    DWORD dwRet;
    dwRet = SetFilePointer(m_hDrive, 0, 0, FILE_BEGIN);
    if (INVALID_SET_FILE_POINTER == dwRet) {
        wxLogMessage("Unable to Set File Pointer - check disk access rights");
        return GetLastError(); // heheh - dovde nece doci nikad :) wxLogFatalError prekida izvrsavanje programa
    }

    int nRet = ReadFile(m_hDrive, &ntfsBS, sizeof (NTFS_PART_BOOT_SEC), &dwBytes, NULL);
    if (!nRet) {
        DWORD dwErrorCode = GetLastError();
        wxLogError("ReadRaw returned error code %d .", dwErrorCode); //sranje - ovde treba provera mesta na hardu i neko lepo preskakanje
        return (dwErrorCode);
    }

    printf("ntfsBS.chOemID: '%4.4s'", ntfsBS.chOemID);

    if (memcmp(ntfsBS.chOemID, "NTFS", 4)) { // check whether it is realy ntfs
        wxLogMessage("Not realy a NTFS drive");
        return ERROR_INVALID_DRIVE;
    }

    /// Cluster is the logical entity
    /// which is made up of several sectors (a physical entity)
    m_dwBytesPerCluster = ntfsBS.bpb.uchSecPerClust * ntfsBS.bpb.wBytesPerSec;

    if (m_puchMFTRecord)
        delete m_puchMFTRecord;

    m_llMFTRecordSz = 0x01 << ((-1)*((char) ntfsBS.bpb.nClustPerMFTRecord));
    m_puchMFTRecord = new BYTE[m_llMFTRecordSz];

    m_bInitialized = true;

    // MFTRecord of MFT is available in the MFTRecord variable
    //   load the entire MFT using it
    nRet = LoadMFT(ntfsBS.bpb.n64MFTLogicalClustNum);
    if (nRet) {
        wxLogFatalError("Loading of MFT failed");
        m_bInitialized = false;
        return nRet;
    }
    return ERROR_SUCCESS;
}

int CNTFSDrive::GetFileDetail(DWORD nFileSeq, t_FileInfo &stFileInfo) {
    int nRet;

    if (!m_bInitialized)
        return (ERROR_INVALID_ACCESS);

    if ((nFileSeq * m_llMFTRecordSz + m_llMFTRecordSz) >= m_dwMFTLen)
        return (ERROR_NO_MORE_FILES);

    CMFTRecord cFile;
    // point the record of the file in the MFT table
    memcpy(m_puchMFTRecord, &m_puchMFT[nFileSeq * m_llMFTRecordSz], m_llMFTRecordSz);

    // read the only file detail not the file data
    cFile.SetDriveHandle(m_hDrive);
    cFile.SetRecordInfo((LONGLONG) m_llStartSector*m_dwBytesPerSector, m_llMFTRecordSz, m_dwBytesPerCluster);
    nRet = cFile.ExtractFile(m_puchMFTRecord, m_llMFTRecordSz, true);
    if (nRet) {
        wxLogMessage("ExtractFile failed");
        return (nRet);
    }
    // set the struct and pass the struct of file detail
    memset(&stFileInfo, 0, sizeof (t_FileInfo));
    wcstombs(stFileInfo.szFilename, (const wchar_t*)cFile.m_attrFilename.wFilename, _MAX_PATH);

    stFileInfo.dwAttributes = cFile.m_attrFilename.dwFlags;

    stFileInfo.n64Create = cFile.m_attrStandard.n64Create;
    stFileInfo.n64Modify = cFile.m_attrStandard.n64Modify;
    stFileInfo.n64Access = cFile.m_attrStandard.n64Access;
    stFileInfo.n64Modfil = cFile.m_attrStandard.n64Modfil;

    stFileInfo.n64Size = cFile.m_attrFilename.n64Allocated;
    stFileInfo.n64Size /= m_dwBytesPerCluster;
    stFileInfo.n64Size = (!stFileInfo.n64Size) ? 1 : stFileInfo.n64Size;
    
    stFileInfo.n64SizeOnDisk = cFile.m_attrFilename.n64Allocated; // Size of the file

    stFileInfo.bDeleted = !cFile.m_bInUse;

    return (ERROR_SUCCESS);
}

int CNTFSDrive::GetFileList(FileInfoHash *m_FileInfoHash) {
    //    t_FileInfo stFInfo;
    t_FileInfo *pFileInfo;
    int nRet, i;

    //    for (i = 0; ((unsigned) i < 0xFFFFFFFF); i++) {
    for (i = 30; ((unsigned) i < 0xFFFFFFFF); i++) { //stavio da i krece od +30 da bi izbegao sistemske MFT recorde

        pFileInfo = new t_FileInfo;
        memset(pFileInfo->szBuffer, '\0', 255);

        nRet = this->GetFileDetail(i, *pFileInfo); // get the file detail one by one, 
        if ((nRet == ERROR_NO_MORE_FILES) || (nRet == ERROR_INVALID_PARAMETER)) {
            wxLogMessage("No more files to add to the list");
            return (0); // ovde smisli sta ces - ne moze ovako da ostane :)
        }

        strcpy(pFileInfo->szBuffer, "");

        if (pFileInfo->dwAttributes & FILE_ATTRIBUTE_READONLY)
            strcat(pFileInfo->szBuffer, "Read Only-");

        if (pFileInfo->dwAttributes & FILE_ATTRIBUTE_HIDDEN)
            strcat(pFileInfo->szBuffer, "Hidden-");

        if (pFileInfo->dwAttributes & FILE_ATTRIBUTE_SYSTEM)
            strcat(pFileInfo->szBuffer, "System File-");

        if (pFileInfo->dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
            strcat(pFileInfo->szBuffer, "Directory-");

//        if (pFileInfo->dwAttributes & FILE_ATTRIBUTE_ARCHIVE)
//            strcat(pFileInfo->szBuffer, "Archive-");

        if (pFileInfo->dwAttributes & FILE_ATTRIBUTE_ENCRYPTED)
            strcat(pFileInfo->szBuffer, "Encrypted-");

        if (pFileInfo->dwAttributes & FILE_ATTRIBUTE_NORMAL)
            strcat(pFileInfo->szBuffer, "Normal-");

        if (pFileInfo->dwAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
            strcat(pFileInfo->szBuffer, "Sparse File-");

        if (pFileInfo->dwAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            strcat(pFileInfo->szBuffer, "Reparse Point-");

        if (pFileInfo->dwAttributes & FILE_ATTRIBUTE_COMPRESSED)
            strcat(pFileInfo->szBuffer, "Compressed-");

        if (pFileInfo->dwAttributes & FILE_ATTRIBUTE_OFFLINE)
            strcat(pFileInfo->szBuffer, "Offline-");

        if (pFileInfo->dwAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)
            strcat(pFileInfo->szBuffer, "Indexed-");

        if (pFileInfo->dwAttributes & 0x4000)
            strcat(pFileInfo->szBuffer, "Encrypted-"); // if it is encrypted

        if (pFileInfo->dwAttributes & 0x10000000)
            strcat(pFileInfo->szBuffer, "Directory-"); // if it is directory

        if (pFileInfo->dwAttributes & 0x10000000)
            strcat(pFileInfo->szBuffer, "Indexed-"); // if it is indexed

        if (pFileInfo->bDeleted)
            strcat(pFileInfo->szBuffer, "Deleted");
        //                // strcpy(szBuffer,"");
        //                if (1 /*!stFInfo.bDeleted */) {
        //                    //dwDeleted++;
        //                    printf("stFInfo.szFilename %s: szBuffer: %s\n", stFInfo.szFilename, szBuffer);
        //        
        //                }
        //                                                             
        if (pFileInfo->bDeleted
                && strlen(pFileInfo->szFilename)//obrisan i ima ime fajla
                && !(pFileInfo->dwAttributes & 0x10000000) //Check if it is a DIR - don-t put DIRs
                && !(pFileInfo->dwAttributes & FILE_ATTRIBUTE_SYSTEM)
                && !(pFileInfo->dwAttributes & FILE_ATTRIBUTE_HIDDEN)
                && !(pFileInfo->dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
                ) {
            pFileInfo->iRecordId = i;
            (*m_FileInfoHash)[i] = pFileInfo;
            pFileInfo = NULL;
        } else {
            delete pFileInfo;
        }
        //        printf("i = %d\n", i);
    }
    return (ERROR_SUCCESS);
}

int CNTFSDrive::RetreiveAndWriteFileToDestination(int nFileSeq, char* sDestintationFileName) {

    if (!m_bInitialized) {
        wxLogMessage("Invalid Access during retrieving \'%s\'", sDestintationFileName);
        return (ERROR_INVALID_ACCESS);
    }

    if ((nFileSeq * m_llMFTRecordSz + m_llMFTRecordSz) >= m_dwMFTLen) {
        wxLogMessage("No more files");
        return (ERROR_NO_MORE_FILES);
    }

    CMFTRecord cFile;
    // point the record of the file in the MFT table
    memcpy(m_puchMFTRecord, &m_puchMFT[nFileSeq * m_llMFTRecordSz], m_llMFTRecordSz);

    // read the only file detail not the file data
    cFile.SetDriveHandle(m_hDrive);
    cFile.SetRecordInfo((LONGLONG) m_llStartSector*m_dwBytesPerSector, m_llMFTRecordSz, m_dwBytesPerCluster);

    wchar_t wDestintationFileName[512];
    memset(wDestintationFileName, '\0', 512 * sizeof (wchar_t));
    mbstowcs(wDestintationFileName, sDestintationFileName, strlen(sDestintationFileName));

    HANDLE hFile = CreateFile(wDestintationFileName, // name of the write
            GENERIC_WRITE | GENERIC_READ, // open for writing and reading
            //            0, // do not share
            FILE_SHARE_READ,
            NULL, // default security
            CREATE_NEW, // create new file only
            FILE_ATTRIBUTE_NORMAL, // normal file
            NULL); // no attr. template

    if (hFile == INVALID_HANDLE_VALUE) {
        wxLogFatalError("Unable to open file \"%s\" for write.", sDestintationFileName);
        DWORD dwErrorCode = GetLastError();
        wxLogError("MapViewOfFile returned error code %d . Possibly not enough drive space", dwErrorCode); //sranje - ovde treba provera mesta na hardu i neko lepo preskakanje
        return (dwErrorCode);
    }

    wxLogMessage("Extracting file \'%s\'.", sDestintationFileName);

    int nRet;
    nRet = cFile.ExtractFile(m_puchMFTRecord, m_llMFTRecordSz, false, hFile);
    if (nRet) {
        wxLogMessage("Extracting file \'%s\' FAILED!!!", sDestintationFileName);
        return (nRet);
    }

    wxLogMessage("Extracted a file.");

    CloseHandle(hFile);

    return (ERROR_SUCCESS);
}
