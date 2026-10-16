
// NTFSDrive.cpp: implementation of the CNTFSDrive class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include <windows.h>
#include <wx/wx.h>
#include <wx/hashmap.h>
#include <map>
#include <new>
#include <string>
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

    m_llMFTSize = 0;
    m_llMFTCacheStart = 0;
    m_dwMFTCacheLen = 0;

    m_llStartSector = 0;
}

bool CNTFSDrive::IsInitialised() {
    return this->m_bInitialized;
}

CNTFSDrive::~CNTFSDrive() {
    delete[] m_puchMFTRecord;
    m_puchMFTRecord = 0;
}

// the boot sector stores the MFT record size as clusters per record when positive,
// or as 2^(-value) bytes when negative (the usual case - e.g. 0xF6 = -10 = 1024 bytes)

static LONGLONG MFTRecordSizeFromBootSector(signed char chClustPerMFTRecord, DWORD dwBytesPerCluster) {
    if (chClustPerMFTRecord > 0)
        return (LONGLONG) chClustPerMFTRecord * dwBytesPerCluster;
    return (LONGLONG) 1 << (-chClustPerMFTRecord);
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

    delete[] m_puchMFTRecord;
    m_puchMFTRecord = 0;

    m_llMFTRecordSz = MFTRecordSizeFromBootSector((signed char) ntfsBS.bpb.nClustPerMFTRecord, m_dwBytesPerCluster);
    if (m_llMFTRecordSz < 256 || m_llMFTRecordSz > 65536) {
        wxLogMessage("Invalid MFT record size in the boot sector.");
        return ERROR_INVALID_DRIVE;
    }
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

    // now extract the MFT record just like the other MFT table records - only where its data is, not the data
    cMFTRec.SetDriveHandle(m_hDrive);
    cMFTRec.SetRecordInfo((LONGLONG) m_llStartSector*m_dwBytesPerSector, m_llMFTRecordSz, m_dwBytesPerCluster);
    nRet = cMFTRec.ExtractFile(m_puchMFTRecord, dwBytes, true);
    if (nRet)
        return nRet;

    if (memcmp(cMFTRec.m_attrFilename.wFilename, uszMFTName, 8))
        return ERROR_BAD_DEVICE; // no MFT file available

    if (cMFTRec.m_vDataRuns.empty()) {
        wxLogMessage("The $MFT record has no readable data runs.");
        return ERROR_BAD_DEVICE;
    }

    m_vMFTRuns = cMFTRec.m_vDataRuns;
    m_llMFTSize = cMFTRec.m_llDataSize;

    LONGLONG llCovered = 0;
    for (size_t i = 0; i < m_vMFTRuns.size(); i++)
        llCovered += m_vMFTRuns[i].llClusters * m_dwBytesPerCluster;

    if (m_llMFTSize > llCovered) {
        // the rest of the run list is in another MFT record (attribute list) - scan what can be found
        wxLogMessage("The MFT continues in another record - only the first %" wxLongLongFmtSpec "d of %"
                wxLongLongFmtSpec "d bytes can be scanned.", llCovered, m_llMFTSize);
        m_llMFTSize = llCovered;
    }
    m_llMFTSize -= m_llMFTSize % m_llMFTRecordSz;

    // cache window: 1 MB, a whole number of clusters (and so of sectors and records)
    DWORD dwCacheSize = (1024 * 1024 / m_dwBytesPerCluster) * m_dwBytesPerCluster;
    if (dwCacheSize < m_dwBytesPerCluster)
        dwCacheSize = m_dwBytesPerCluster;
    if (dwCacheSize < m_llMFTRecordSz)
        dwCacheSize = (DWORD) m_llMFTRecordSz;
    m_vMFTCache.resize(dwCacheSize);
    m_dwMFTCacheLen = 0;

    wxLogMessage("MFT: %" wxLongLongFmtSpec "d bytes in %u fragments, %" wxLongLongFmtSpec "d records.",
            m_llMFTSize, (unsigned) m_vMFTRuns.size(), m_llMFTSize / m_llMFTRecordSz);
    return ERROR_SUCCESS;
}

// read dwLen bytes of a non-resident stream (the MFT, $Bitmap...), starting at stream byte llOffset
// (a whole number of clusters), following its fragments

int CNTFSDrive::ReadRunsBytes(const std::vector<NTFS_DATA_RUN> &vRuns, LONGLONG llOffset, BYTE *puchOut, DWORD dwLen) {
    LONGLONG llRunStart = 0; // stream byte offset where the current run starts

    for (size_t i = 0; i < vRuns.size() && dwLen; i++) {
        const NTFS_DATA_RUN &run = vRuns[i];
        LONGLONG llRunBytes = run.llClusters * m_dwBytesPerCluster;

        if (llOffset < llRunStart + llRunBytes) {
            LONGLONG llInRun = llOffset - llRunStart;
            DWORD dwPiece = (llRunBytes - llInRun < dwLen) ? (DWORD) (llRunBytes - llInRun) : dwLen;

            if (run.bSparse) {
                memset(puchOut, 0, dwPiece);
            } else {
                LARGE_INTEGER pos;
                pos.QuadPart = m_llStartSector * m_dwBytesPerSector + run.llLCN * m_dwBytesPerCluster + llInRun;
                if (!SetFilePointerEx(m_hDrive, pos, NULL, FILE_BEGIN))
                    return GetLastError();

                DWORD dwDone = 0;
                while (dwDone < dwPiece) {
                    DWORD dwBytes = 0;
                    if (!ReadFile(m_hDrive, puchOut + dwDone, dwPiece - dwDone, &dwBytes, NULL))
                        return GetLastError();
                    if (!dwBytes)
                        return ERROR_HANDLE_EOF;
                    dwDone += dwBytes;
                }
            }

            puchOut += dwPiece;
            llOffset += dwPiece;
            dwLen -= dwPiece;
        }
        llRunStart += llRunBytes;
    }

    return dwLen ? ERROR_HANDLE_EOF : ERROR_SUCCESS;
}

// copy MFT record nFileSeq into m_puchMFTRecord, reading the MFT through the cache window

int CNTFSDrive::LoadMFTRecord(DWORD nFileSeq) {
    LONGLONG llOffset = (LONGLONG) nFileSeq * m_llMFTRecordSz;
    if (llOffset + m_llMFTRecordSz > m_llMFTSize)
        return ERROR_NO_MORE_FILES;

    if (!m_dwMFTCacheLen || llOffset < m_llMFTCacheStart || llOffset + m_llMFTRecordSz > m_llMFTCacheStart + m_dwMFTCacheLen) {
        DWORD dwCacheSize = (DWORD) m_vMFTCache.size();
        m_llMFTCacheStart = llOffset - llOffset % dwCacheSize;

        // the last window may be shorter - round it up to whole clusters, the MFT's clusters are all allocated
        LONGLONG llLeft = m_llMFTSize - m_llMFTCacheStart;
        DWORD dwRead = (llLeft < dwCacheSize) ? (DWORD) llLeft : dwCacheSize;
        dwRead = ((dwRead + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster) * m_dwBytesPerCluster;
        if (dwRead > dwCacheSize)
            dwRead = dwCacheSize;

        int nRet = ReadRunsBytes(m_vMFTRuns, m_llMFTCacheStart, &m_vMFTCache[0], dwRead);
        if (nRet) {
            // unreadable part of the MFT (bad sectors?) - its records read as empty and are skipped
            wxLogMessage("Cannot read the MFT at offset %" wxLongLongFmtSpec "d (error %d) - %u records skipped.",
                    m_llMFTCacheStart, nRet, (unsigned) (dwRead / m_llMFTRecordSz));
            memset(&m_vMFTCache[0], 0, dwRead);
        }
        m_dwMFTCacheLen = dwRead;
    }

    memcpy(m_puchMFTRecord, &m_vMFTCache[(size_t) (llOffset - m_llMFTCacheStart)], m_llMFTRecordSz);
    return ERROR_SUCCESS;
}

// $Bitmap is MFT record 6: one bit per cluster of the volume, set = cluster in use.
// About 32 MB per TB with 4 KB clusters - read once, sequentially, at the start of a scan.

#define MAX_VOLUME_BITMAP_SIZE (512 * 1024 * 1024)

int CNTFSDrive::LoadVolumeBitmap() {
    std::vector<BYTE>().swap(m_vVolumeBitmap); // free the previous scan's bitmap

    int nRet = LoadMFTRecord(6);
    if (nRet)
        return nRet;

    CMFTRecord cBitmap;
    cBitmap.SetDriveHandle(m_hDrive);
    cBitmap.SetRecordInfo((LONGLONG) m_llStartSector*m_dwBytesPerSector, m_llMFTRecordSz, m_dwBytesPerCluster);
    nRet = cBitmap.ExtractFile(m_puchMFTRecord, m_llMFTRecordSz, true);
    if (nRet)
        return nRet;
    if (cBitmap.m_vDataRuns.empty() || cBitmap.m_llDataSize <= 0)
        return ERROR_FILE_CORRUPT;

    if (cBitmap.m_llDataSize > MAX_VOLUME_BITMAP_SIZE) {
        wxLogMessage("The volume bitmap is too large to load (%" wxLongLongFmtSpec "d bytes) - recoverability will be unknown.",
                cBitmap.m_llDataSize);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    // whole clusters - the volume can only be read in whole sectors
    DWORD dwRead = (DWORD) (((cBitmap.m_llDataSize + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster) * m_dwBytesPerCluster);
    try {
        m_vVolumeBitmap.resize(dwRead);
    } catch (std::bad_alloc &) {
        wxLogMessage("Not enough memory for the volume bitmap - recoverability will be unknown.");
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    nRet = ReadRunsBytes(cBitmap.m_vDataRuns, 0, &m_vVolumeBitmap[0], dwRead);
    if (nRet) {
        wxLogMessage("Cannot read the volume bitmap (error %d) - recoverability will be unknown.", nRet);
        std::vector<BYTE>().swap(m_vVolumeBitmap);
        return nRet;
    }
    m_vVolumeBitmap.resize((size_t) cBitmap.m_llDataSize);

    wxLogMessage("Volume bitmap loaded: %u bytes.", (unsigned) m_vVolumeBitmap.size());
    return ERROR_SUCCESS;
}

// judge whether a deleted file's clusters have been reused by other files since it was deleted

void CNTFSDrive::CheckRecoverability(const CMFTRecord &cFile, t_FileInfo &stFileInfo) {
    stFileInfo.iRecoverability = RECOVERABILITY_UNKNOWN;
    stFileInfo.iPercentIntact = 0;

    if (cFile.m_bDataResident || cFile.m_llDataSize == 0) {
        // the data lives in the MFT record itself, which is still there (or there is no data at all)
        stFileInfo.iRecoverability = RECOVERABILITY_GOOD;
        stFileInfo.iPercentIntact = 100;
        return;
    }
    if (m_vVolumeBitmap.empty() || cFile.m_vDataRuns.empty() || (cFile.m_wDataFlags & 0x0001))
        return; // no bitmap, damaged run list, or compressed (recovery would fail anyway)

    LONGLONG llBitmapClusters = (LONGLONG) m_vVolumeBitmap.size() * 8;
    LONGLONG llNeeded = (cFile.m_llDataSize + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster;
    LONGLONG llTotal = 0, llFree = 0;
    const BYTE *puchBitmap = &m_vVolumeBitmap[0];

    for (size_t i = 0; i < cFile.m_vDataRuns.size() && llNeeded > 0; i++) {
        const NTFS_DATA_RUN &run = cFile.m_vDataRuns[i];
        LONGLONG llCount = (run.llClusters < llNeeded) ? run.llClusters : llNeeded;
        llNeeded -= llCount;
        if (run.bSparse)
            continue; // no clusters on disk - nothing to overwrite

        LONGLONG c = run.llLCN, llEnd = run.llLCN + llCount;
        llTotal += llCount;
        while (c < llEnd) {
            if (c >= llBitmapClusters) { // beyond the volume - can't be right, count as lost
                c = llEnd;
                break;
            }
            if (!(c & 7) && llEnd - c >= 8) { // a whole bitmap byte at once
                BYTE b = puchBitmap[c >> 3];
                if (b == 0)
                    llFree += 8;
                else if (b != 0xFF)
                    for (int bit = 0; bit < 8; bit++)
                        if (!(b & (1 << bit)))
                            llFree++;
                c += 8;
            } else {
                if (!(puchBitmap[c >> 3] & (1 << (c & 7))))
                    llFree++;
                c++;
            }
        }
    }

    if (!llTotal) { // entirely sparse
        stFileInfo.iRecoverability = RECOVERABILITY_GOOD;
        stFileInfo.iPercentIntact = 100;
        return;
    }

    stFileInfo.iPercentIntact = (int) (llFree * 100 / llTotal);
    if (llFree && llFree < llTotal) // never show a partial file as 0% or 100%
        stFileInfo.iPercentIntact = (stFileInfo.iPercentIntact < 1) ? 1 : (stFileInfo.iPercentIntact > 99 ? 99 : stFileInfo.iPercentIntact);
    if (llFree == llTotal)
        stFileInfo.iRecoverability = RECOVERABILITY_GOOD;
    else if (llFree == 0)
        stFileInfo.iRecoverability = RECOVERABILITY_OVERWRITTEN;
    else
        stFileInfo.iRecoverability = RECOVERABILITY_PARTIAL;
}


/// this function if succeeded it will allocate the buffer and passed to the caller
//    the caller's responsibility to free it

int CNTFSDrive::Read_File(DWORD nFileSeq, BYTE *&puchFileData, DWORD &dwFileDataLen) {
    int nRet;

    if (!m_bInitialized) {
        wxLogError("Calling Read_File for a drive that is not initialized.");
        return ERROR_INVALID_ACCESS;
    }

    // point the record of the file in the MFT table
    nRet = LoadMFTRecord(nFileSeq);
    if (nRet)
        return nRet;

    CMFTRecord cFile;

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

    delete[] m_puchMFTRecord;
    m_puchMFTRecord = 0;

    m_llMFTRecordSz = MFTRecordSizeFromBootSector((signed char) ntfsBS.bpb.nClustPerMFTRecord, m_dwBytesPerCluster);
    if (m_llMFTRecordSz < 256 || m_llMFTRecordSz > 65536) {
        wxLogMessage("Invalid MFT record size in the boot sector.");
        return ERROR_INVALID_DRIVE;
    }
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

    // point the record of the file in the MFT table
    nRet = LoadMFTRecord(nFileSeq);
    if (nRet)
        return nRet;

    CMFTRecord cFile;

    // read the only file detail not the file data
    cFile.SetDriveHandle(m_hDrive);
    cFile.SetRecordInfo((LONGLONG) m_llStartSector*m_dwBytesPerSector, m_llMFTRecordSz, m_dwBytesPerCluster);
    nRet = cFile.ExtractFile(m_puchMFTRecord, m_llMFTRecordSz, true);
    if (nRet) {
        if (nRet != ERROR_FILE_INVALID) // unused (never written) records are normal - don't flood the log
            wxLogMessage("ExtractFile failed for MFT record %u (error %d)", nFileSeq, nRet);
        return (nRet);
    }
    // set the struct and pass the struct of file detail
    memset(&stFileInfo, 0, sizeof (t_FileInfo));
    // keep the name as UTF-16 - converting it to char loses every character outside the ANSI code page
    wcsncpy(stFileInfo.szFilename, (const wchar_t*) cFile.m_attrFilename.wFilename, _MAX_PATH - 1);

    stFileInfo.dwAttributes = cFile.m_attrFilename.dwFlags;

    stFileInfo.n64Create = cFile.m_attrStandard.n64Create;
    stFileInfo.n64Modify = cFile.m_attrStandard.n64Modify;
    stFileInfo.n64Access = cFile.m_attrStandard.n64Access;
    stFileInfo.n64Modfil = cFile.m_attrStandard.n64Modfil;

    stFileInfo.n64Size = cFile.m_attrFilename.n64Allocated;
    stFileInfo.n64Size /= m_dwBytesPerCluster;
    stFileInfo.n64Size = (!stFileInfo.n64Size) ? 1 : stFileInfo.n64Size;
    
    stFileInfo.n64SizeOnDisk = cFile.m_attrFilename.n64Allocated; // Size of the file
    stFileInfo.n64RealSize = cFile.m_llDataSize;

    stFileInfo.bDeleted = !cFile.m_bInUse;
    if (stFileInfo.bDeleted)
        CheckRecoverability(cFile, stFileInfo);

    return (ERROR_SUCCESS);
}

int CNTFSDrive::GetFileList(FileInfoHash *m_FileInfoHash) {
    //    t_FileInfo stFInfo;
    t_FileInfo *pFileInfo;
    int nRet, i;

    // which clusters are in use right now - to tell which deleted files are probably still intact
    if (LoadVolumeBitmap())
        wxLogMessage("Volume bitmap not available - recoverability of the files will be unknown.");

    //    for (i = 0; ((unsigned) i < 0xFFFFFFFF); i++) {
    for (i = 30; ((unsigned) i < 0xFFFFFFFF); i++) { //stavio da i krece od +30 da bi izbegao sistemske MFT recorde

        pFileInfo = new t_FileInfo;
        memset(pFileInfo->szBuffer, '\0', 255);

        nRet = this->GetFileDetail(i, *pFileInfo); // get the file detail one by one, 
        if ((nRet == ERROR_NO_MORE_FILES) || (nRet == ERROR_INVALID_ACCESS)) {
            delete pFileInfo;
            wxLogMessage("No more files to add to the list");
            std::vector<BYTE>().swap(m_vVolumeBitmap); // only needed during the scan
            ResolveRecycleBinNames(m_FileInfoHash);
            return (0); // ovde smisli sta ces - ne moze ovako da ostane :)
        }
        if (nRet) { // unused or damaged record - skip it, keep scanning the rest of the MFT
            delete pFileInfo;
            continue;
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
                && wcslen(pFileInfo->szFilename)//obrisan i ima ime fajla
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

// Recycle Bin (Vista and later): a deleted file is renamed to $R<6 random chars><original extension>
// and a small companion file $I<same 6 chars><extension> holds its original full path:
//   version 1 (Vista - 8.1): 8 bytes version, 8 file size, 8 deletion time, path as 260 WCHARs
//   version 2 (Windows 10+): 8 bytes version, 8 file size, 8 deletion time, 4 path length in WCHARs, path

static bool GetOriginalPathFromRecycleBinInfo(const BYTE *puchData, DWORD dwLen, std::wstring &wsPath) {
    if (dwLen < 24)
        return false;

    LONGLONG llVersion;
    memcpy(&llVersion, puchData, sizeof (llVersion));

    const wchar_t *pwPath;
    DWORD dwMaxChars;
    if (llVersion == 1) {
        pwPath = (const wchar_t *) (puchData + 24);
        dwMaxChars = (dwLen - 24) / sizeof (wchar_t);
        if (dwMaxChars > MAX_PATH)
            dwMaxChars = MAX_PATH;
    } else if (llVersion == 2) {
        if (dwLen < 28)
            return false;
        memcpy(&dwMaxChars, puchData + 24, sizeof (DWORD));
        if (dwMaxChars > (dwLen - 28) / sizeof (wchar_t))
            return false;
        pwPath = (const wchar_t *) (puchData + 28);
    } else
        return false;

    DWORD dwChars = 0;
    while (dwChars < dwMaxChars && pwPath[dwChars])
        dwChars++;
    if (!dwChars)
        return false;

    wsPath.assign(pwPath, dwChars);
    return true;
}

// replace "$R...." names with the original file names stored in the matching "$I...." files

void CNTFSDrive::ResolveRecycleBinNames(FileInfoHash *finfohash) {
    FileInfoHash::iterator it;

    std::map<std::wstring, int> mapInfoFiles; // "$I...." name -> MFT record
    for (it = finfohash->begin(); it != finfohash->end(); ++it)
        if (!wcsncmp(it->second->szFilename, L"$I", 2))
            mapInfoFiles[it->second->szFilename] = it->second->iRecordId;

    for (it = finfohash->begin(); it != finfohash->end(); ++it) {
        t_FileInfo *pFileInfo = it->second;
        if (wcsncmp(pFileInfo->szFilename, L"$R", 2) || wcslen(pFileInfo->szFilename) < 8)
            continue;

        wxLogMessage("A file deleted from Recycle Bin is found: %s", wxString(pFileInfo->szFilename));

        std::wstring wsInfoName = std::wstring(L"$I") + (pFileInfo->szFilename + 2);
        std::map<std::wstring, int>::iterator itInfo = mapInfoFiles.find(wsInfoName);
        if (itInfo == mapInfoFiles.end()) {
            wxLogMessage("No %s found - the original name is lost.", wxString(wsInfoName.c_str()));
            continue;
        }

        BYTE *puchData = 0;
        DWORD dwLen = 0;
        if (Read_File(itInfo->second, puchData, dwLen)) {
            wxLogMessage("Error when trying to retrieve contents of %s", wxString(wsInfoName.c_str()));
            continue;
        }

        std::wstring wsPath;
        bool bFound = GetOriginalPathFromRecycleBinInfo(puchData, dwLen, wsPath);
        delete[] puchData;
        if (!bFound) {
            wxLogMessage("%s has an unknown format.", wxString(wsInfoName.c_str()));
            continue;
        }

        size_t nSlash = wsPath.find_last_of(L'\\');
        std::wstring wsName = (nSlash == std::wstring::npos) ? wsPath : wsPath.substr(nSlash + 1);
        if (wsName.empty())
            continue;

        wcsncpy(pFileInfo->szFilename, wsName.c_str(), _MAX_PATH - 1);
        pFileInfo->szFilename[_MAX_PATH - 1] = 0;
        strcpy(pFileInfo->szBuffer, "Deleted in Recycle Bin");
        wxLogMessage("A file name retrieved from Recycle Bin: %s", wxString(pFileInfo->szFilename));
    }
}

int CNTFSDrive::RetreiveAndWriteFileToDestination(int nFileSeq, const wchar_t *wszDestinationFileName) {
    wxString sDestintationFileName(wszDestinationFileName); // for logging

    if (!m_bInitialized) {
        wxLogMessage("Invalid Access during retrieving \'%s\'", sDestintationFileName);
        return (ERROR_INVALID_ACCESS);
    }

    // point the record of the file in the MFT table
    int nRet = LoadMFTRecord(nFileSeq);
    if (nRet) {
        wxLogMessage("No more files");
        return (nRet);
    }

    CMFTRecord cFile;

    // read the only file detail not the file data
    cFile.SetDriveHandle(m_hDrive);
    cFile.SetRecordInfo((LONGLONG) m_llStartSector*m_dwBytesPerSector, m_llMFTRecordSz, m_dwBytesPerCluster);

    HANDLE hFile = CreateFileW(wszDestinationFileName, // name of the write
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

    nRet = cFile.ExtractFile(m_puchMFTRecord, m_llMFTRecordSz, false, hFile);
    if (nRet) {
        wxLogMessage("Extracting file \'%s\' FAILED!!!", sDestintationFileName);
        CloseHandle(hFile);
        return (nRet);
    }

    wxLogMessage("Extracted a file.");

    CloseHandle(hFile);

    return (ERROR_SUCCESS);
}
