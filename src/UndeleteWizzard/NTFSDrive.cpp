
// NTFSDrive.cpp: implementation of the CNTFSDrive class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include <windows.h>
#include <wx/wx.h>
#include <wx/hashmap.h>
#include <algorithm>
#include <map>
#include <new>
#include <string>
#include "NTFSDrive.h"
#include "MFTRecord.h"
#include "FileContents.h"

// MFT record of the root folder of every NTFS volume
#define ROOT_FOLDER_RECORD 5

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
    if (INVALID_SET_FILE_POINTER == dwCur && GetLastError() != NO_ERROR) {
        DWORD dwErrorCode = GetLastError();
        wxLogMessage("Unable to Set File Pointer (error %u) - check disk access rights", (unsigned) dwErrorCode);
        return dwErrorCode;
    }

    // Read the boot sector for the MFT infomation - 4 KB: a volume handle only reads whole sectors, and on a
    // drive with 4096-byte sectors a 512-byte read fails
    std::vector<BYTE> vBoot(4096);
    int nRet = ReadFile(m_hDrive, &vBoot[0], (DWORD) vBoot.size(), &dwBytes, NULL);
    if (!nRet || dwBytes < sizeof (NTFS_PART_BOOT_SEC)) {
        DWORD dwErrorCode = nRet ? ERROR_HANDLE_EOF : GetLastError();
        wxLogMessage("Unable to read the boot sector (error %u) - check disk access rights.", (unsigned) dwErrorCode);
        return (dwErrorCode);
    }
    memcpy(&ntfsBS, &vBoot[0], sizeof (NTFS_PART_BOOT_SEC));

    if (memcmp(ntfsBS.chOemID, "NTFS", 4)) // check whether it is realy ntfs
        return (ERROR_INVALID_DRIVE);

    /// Cluster is the logical entity
    ///  which is made up of several sectors (a physical entity). Clusters over 64 KB (Windows 10 1709 on: up to
    ///  2 MB) don't fit the byte - it holds a negative power of two then: 0xF8 = -8 = 2^8 sectors.
    DWORD dwSectorsPerCluster = ntfsBS.bpb.uchSecPerClust;
    if (dwSectorsPerCluster > 0x80)
        dwSectorsPerCluster = (DWORD) 1 << (256 - dwSectorsPerCluster);
    m_dwBytesPerCluster = dwSectorsPerCluster * ntfsBS.bpb.wBytesPerSec;
    if (!ntfsBS.bpb.wBytesPerSec || !m_dwBytesPerCluster || m_dwBytesPerCluster > 2 * 1024 * 1024) {
        wxLogMessage("Invalid cluster size in the boot sector.");
        return ERROR_INVALID_DRIVE;
    }

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
    if (dwRet == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
        DWORD dwErrorCode = GetLastError();
        wxLogMessage("Unable to Set File Pointer (error %u) - check disk access rights", (unsigned) dwErrorCode);
        return dwErrorCode;
    }

    /// reading the first record in the NTFS table.
    //   the first record in the NTFS is always MFT record. Read in whole clusters - a cluster is always a whole
    //   number of sectors, a 1 KB record isn't on a drive with 4096-byte sectors.
    DWORD dwFirstRead = (DWORD) (((m_llMFTRecordSz + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster) * m_dwBytesPerCluster);
    std::vector<BYTE> vFirst(dwFirstRead);
    nRet = ReadFile(m_hDrive, &vFirst[0], dwFirstRead, &dwBytes, NULL);
    if (!nRet || dwBytes < m_llMFTRecordSz) {
        DWORD dwErrorCode = nRet ? ERROR_HANDLE_EOF : GetLastError();
        wxLogMessage("Unable to read the first MFT record (error %u).", (unsigned) dwErrorCode);
        return (dwErrorCode);
    }
    memcpy(m_puchMFTRecord, &vFirst[0], (size_t) m_llMFTRecordSz);

    // now extract the MFT record just like the other MFT table records - only where its data is, not the data
    cMFTRec.SetDriveHandle(m_hDrive);
    cMFTRec.SetRecordInfo((LONGLONG) m_llStartSector*m_dwBytesPerSector, m_llMFTRecordSz, m_dwBytesPerCluster);
    nRet = cMFTRec.ExtractFile(m_puchMFTRecord, m_llMFTRecordSz, true);
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
    bool bStartChecked = false;

    for (size_t i = 0; i < cFile.m_vDataRuns.size() && llNeeded > 0; i++) {
        const NTFS_DATA_RUN &run = cFile.m_vDataRuns[i];
        LONGLONG llCount = (run.llClusters < llNeeded) ? run.llClusters : llNeeded;
        llNeeded -= llCount;
        if (run.bSparse)
            continue; // no clusters on disk - nothing to overwrite

        if (!bStartChecked) { // the first cluster holds the file's header - the content check needs it
            bStartChecked = true;
            stFileInfo.bStartReused = run.llLCN >= llBitmapClusters || (puchBitmap[run.llLCN >> 3] & (1 << (run.llLCN & 7)));
        }

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

int CNTFSDrive::ReadFileStart(DWORD nFileSeq, std::vector<BYTE> &vData, DWORD dwMax) {
    vData.clear();
    if (!m_bInitialized)
        return ERROR_INVALID_ACCESS;

    int nRet = LoadMFTRecord(nFileSeq);
    if (nRet)
        return nRet;

    CMFTRecord cFile;
    cFile.SetDriveHandle(m_hDrive);
    cFile.SetRecordInfo((LONGLONG) m_llStartSector*m_dwBytesPerSector, m_llMFTRecordSz, m_dwBytesPerCluster);
    nRet = cFile.ExtractFile(m_puchMFTRecord, m_llMFTRecordSz, true); // where the data is, not the data
    if (nRet)
        return nRet;
    if (cFile.IsEncrypted())
        return ERROR_FILE_ENCRYPTED;

    if (cFile.m_bDataResident) {
        // the data is in the record itself - extract it from a fresh copy (the fixups were applied to this one)
        nRet = LoadMFTRecord(nFileSeq);
        if (!nRet)
            nRet = cFile.ExtractFile(m_puchMFTRecord, m_llMFTRecordSz);
        if (nRet)
            return nRet;
        DWORD dwLen = (cFile.m_llFileDataSz < dwMax) ? (DWORD) cFile.m_llFileDataSz : dwMax;
        if (dwLen)
            vData.assign(cFile.m_puchFileData, cFile.m_puchFileData + dwLen);
        return ERROR_SUCCESS;
    }

    if (cFile.m_wDataFlags & 0x0001)
        return ERROR_NOT_SUPPORTED; // compressed - the clusters don't hold the plain data
    if (cFile.m_llDataSize <= 0)
        return ERROR_SUCCESS;
    if (cFile.m_vDataRuns.empty())
        return ERROR_NOT_SUPPORTED; // damaged run list, or it starts in another record

    DWORD dwWant = (cFile.m_llDataSize < dwMax) ? (DWORD) cFile.m_llDataSize : dwMax;
    DWORD dwRead = ((dwWant + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster) * m_dwBytesPerCluster; // whole clusters
    vData.resize(dwRead);
    nRet = ReadRunsBytes(cFile.m_vDataRuns, 0, &vData[0], dwRead);
    if (nRet) {
        vData.clear();
        return nRet;
    }
    vData.resize(dwWant);
    return ERROR_SUCCESS;
}

// open a drive letter ("E:" or "E:\") as a raw volume and load its NTFS structures

int CNTFSDrive::ReadVolume(wxString wxsSelectedDir) {
    m_bInitialized = false;
    int nRet = OpenVolume(wxsSelectedDir);
    if (nRet)
        return nRet;

    // the volume starts at byte 0 of its own handle, so the sector size only matters for multiplying with 0
    DWORD dwSectorsPerCluster, dwBytesPerSector = 512, dwFreeClusters, dwTotalClusters;
    GetDiskFreeSpaceW((wxsSelectedDir.Left(2) + "\\").wc_str(), &dwSectorsPerCluster, &dwBytesPerSector, &dwFreeClusters, &dwTotalClusters);
    SetStartSector(0, dwBytesPerSector);

    nRet = Initialize();
    if (nRet)
        wxLogMessage("%s: no NTFS volume, or unreadable (error %d).", wxsSelectedDir, nRet);
    return nRet;
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
    stFileInfo = t_FileInfo();
    // keep the name as UTF-16 - converting it to char loses every character outside the ANSI code page
    stFileInfo.sFilename.assign((const wchar_t*) cFile.m_attrFilename.wFilename);

    // FILE_NAME's copy of the attributes is only updated with the name - encryption and compression come from
    // STANDARD_INFORMATION, which is current
    stFileInfo.dwAttributes = cFile.m_attrFilename.dwFlags
            | (cFile.m_attrStandard.dwFATAttributes & (FILE_ATTRIBUTE_ENCRYPTED | FILE_ATTRIBUTE_COMPRESSED));

    stFileInfo.n64Create = cFile.m_attrStandard.n64Create;
    stFileInfo.n64Modify = cFile.m_attrStandard.n64Modify;
    stFileInfo.n64Access = cFile.m_attrStandard.n64Access;
    stFileInfo.n64Modfil = cFile.m_attrStandard.n64Modfil;

    stFileInfo.n64SizeOnDisk = cFile.m_attrFilename.n64Allocated; // Size of the file
    stFileInfo.n64RealSize = cFile.m_llDataSize;

    // the parent reference: 48-bit record number + the 16-bit sequence number that record had back then
    stFileInfo.dwParentRecord = (DWORD) (cFile.m_attrFilename.dwMftParentDir & 0xFFFFFFFFFFFFLL);
    stFileInfo.wParentSeq = (WORD) ((ULONGLONG) cFile.m_attrFilename.dwMftParentDir >> 48);
    stFileInfo.wSequence = cFile.m_wSequence;
    stFileInfo.bDirectory = cFile.m_bDirectory || (cFile.m_attrFilename.dwFlags & 0x10000000);

    stFileInfo.bDeleted = !cFile.m_bInUse;
    if (stFileInfo.bDeleted && !stFileInfo.bDirectory && !cFile.IsEncrypted()) // encrypted: never recoverable here
        CheckRecoverability(cFile, stFileInfo);

    return (ERROR_SUCCESS);
}

// scan the whole MFT and put every deleted (non-system) file into the map - their folders come in FinishScan.
// Returns ERROR_CANCELLED if the cancel flag was set - the map then holds what was found so far.

int CNTFSDrive::ScanFiles(FileInfoHash *m_FileInfoHash) {
    t_FileInfo stFileInfo;
    int nRet, i;
    std::vector<ST_FOLDER> &vFolders = m_vFolders;
    vFolders.clear();

    // which clusters are in use right now - to tell which deleted files are probably still intact
    if (LoadVolumeBitmap())
        wxLogMessage("Volume bitmap not available - recoverability of the files will be unknown.");

    // records 0-23 are the system files ($MFT, $Bitmap, ...) and reserved ones - the first user file is 24
    for (i = 24; ((unsigned) i < 0xFFFFFFFF); i++) {

        if (!(i & 0xFFF)) {
            if (IsCancelled()) {
                wxLogMessage("Scan cancelled at MFT record %d.", i);
                std::vector<BYTE>().swap(m_vVolumeBitmap);
                return ERROR_CANCELLED; // FinishScan still gives what was found its folders
            }
            LONGLONG llRecords = m_llMFTSize / m_llMFTRecordSz;
            if (llRecords > 0)
                ReportScanProgress((int) wxMin(99LL, (LONGLONG) i * 100 / llRecords));
        }

        nRet = this->GetFileDetail(i, stFileInfo); // get the file detail one by one,
        if ((nRet == ERROR_NO_MORE_FILES) || (nRet == ERROR_INVALID_ACCESS))
            break;
        if (nRet) // unused or damaged record - skip it, keep scanning the rest of the MFT
            continue;

        if (stFileInfo.bDirectory && !stFileInfo.sFilename.empty()) {
            // every folder, in use or deleted - the files' folder paths are built from these after the scan
            ST_FOLDER folder;
            folder.dwRecord = i;
            folder.dwParentRecord = stFileInfo.dwParentRecord;
            folder.wParentSeq = stFileInfo.wParentSeq;
            folder.wSequence = stFileInfo.wSequence;
            folder.bInUse = !stFileInfo.bDeleted;
            folder.sName = stFileInfo.sFilename;
            vFolders.push_back(folder);
            continue;
        }

        if (stFileInfo.bDeleted
                && !stFileInfo.sFilename.empty() //obrisan i ima ime fajla
                && !(stFileInfo.dwAttributes & 0x10000000) //Check if it is a DIR - don-t put DIRs
                && !(stFileInfo.dwAttributes & FILE_ATTRIBUTE_SYSTEM)
                && !(stFileInfo.dwAttributes & FILE_ATTRIBUTE_HIDDEN)
                && !(stFileInfo.dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
                ) {
            stFileInfo.iRecordId = i;
            AddFoundFile(m_FileInfoHash, new t_FileInfo(stFileInfo));
        }
    }

    wxLogMessage("No more files to add to the list (%u folders seen).", (unsigned) vFolders.size());
    std::vector<BYTE>().swap(m_vVolumeBitmap); // only needed during the scan
    return (ERROR_SUCCESS);
}

// the folder paths, from the folders the scan saw - a folder only seen later can complete a file's path

void CNTFSDrive::FinishScan(FileInfoHash *finfohash) {
    ResolveFolders(finfohash, m_vFolders);
    std::vector<ST_FOLDER>().swap(m_vFolders);
    CRecoveryDrive::FinishScan(finfohash); // the Recycle Bin names
}

// builds folder paths from the scan's folder records, remembering each folder's path once

class CFolderPaths {
public:
    typedef CNTFSDrive::ST_FOLDER ST_FOLDER;

    CFolderPaths(const std::vector<ST_FOLDER> &vFolders) : m_vFolders(vFolders) {
    }

    // path of the folder a reference points to: "\" for the root, "?" if that folder can't be known
    const std::wstring &PathOf(DWORD dwRecord, WORD wSeq, int iDepth = 0) {
        static const std::wstring sRoot(L"\\"), sUnknown(L"?");
        if (dwRecord == ROOT_FOLDER_RECORD)
            return sRoot;

        const ST_FOLDER *pFolder = Find(dwRecord);
        if (!pFolder || !StillTheSameFolder(*pFolder, wSeq) || iDepth > 100) // the depth guards against loops
            return sUnknown;

        std::map<DWORD, std::wstring>::iterator it = m_mapPaths.find(dwRecord);
        if (it != m_mapPaths.end())
            return it->second;

        const std::wstring &sParent = PathOf(pFolder->dwParentRecord, pFolder->wParentSeq, iDepth + 1);
        std::wstring sPath = (sParent == sRoot) ? sParent + pFolder->sName : sParent + L"\\" + pFolder->sName;
        return m_mapPaths[dwRecord] = sPath;
    }

private:
    const std::vector<ST_FOLDER> &m_vFolders;
    std::map<DWORD, std::wstring> m_mapPaths;

    struct RecordLess {
        bool operator()(const ST_FOLDER &folder, DWORD dwRecord) const {
            return folder.dwRecord < dwRecord;
        }
    };

    const ST_FOLDER *Find(DWORD dwRecord) const {
        std::vector<ST_FOLDER>::const_iterator it = std::lower_bound(m_vFolders.begin(), m_vFolders.end(), dwRecord, RecordLess());
        return (it != m_vFolders.end() && it->dwRecord == dwRecord) ? &*it : NULL;
    }

    // a record is reused for other files and folders - its sequence number goes up by one each time it is freed
    static bool StillTheSameFolder(const ST_FOLDER &folder, WORD wReferencedSeq) {
        if (!wReferencedSeq || folder.wSequence == wReferencedSeq)
            return true;
        WORD wAfterDelete = (WORD) (wReferencedSeq + 1);
        if (!wAfterDelete)
            wAfterDelete = 1; // 0 is skipped
        return !folder.bInUse && folder.wSequence == wAfterDelete; // the folder was deleted too, record not reused
    }
};

void CNTFSDrive::ResolveFolders(FileInfoHash *finfohash, const std::vector<ST_FOLDER> &vFolders) {
    CFolderPaths paths(vFolders);
    for (FileInfoHash::iterator it = finfohash->begin(); it != finfohash->end(); ++it)
        it->second->sFolder = paths.PathOf(it->second->dwParentRecord, it->second->wParentSeq);
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

    HANDLE hFile = CreateFileW(LongPathForWindows(sDestintationFileName).wc_str(), // name of the write
            GENERIC_WRITE | GENERIC_READ, // open for writing and reading
            //            0, // do not share
            FILE_SHARE_READ,
            NULL, // default security
            CREATE_NEW, // create new file only
            FILE_ATTRIBUTE_NORMAL, // normal file
            NULL); // no attr. template

    if (hFile == INVALID_HANDLE_VALUE) {
        // one file that can't be created must not stop the others - the copy thread logs it and goes on
        DWORD dwErrorCode = GetLastError();
        wxLogMessage("Unable to create file \"%s\" (error %u).", sDestintationFileName, (unsigned) dwErrorCode);
        return dwErrorCode ? (int) dwErrorCode : ERROR_CANNOT_MAKE;
    }

    wxLogMessage("Extracting file \'%s\'.", sDestintationFileName);

    cFile.SetCancelFlag(m_pbCancel);
    cFile.SetProgressCounter(m_plProgressKB);
    nRet = cFile.ExtractFile(m_puchMFTRecord, m_llMFTRecordSz, false, hFile);
    if (nRet) {
        wxLogMessage("Extracting file \'%s\' FAILED (error %d)!!!", sDestintationFileName, nRet);
        CloseHandle(hFile);
        // a cut-off file would look like a recovered one; an encrypted or compressed one wasn't written at all
        if (nRet == ERROR_CANCELLED || nRet == ERROR_FILE_ENCRYPTED || nRet == ERROR_NOT_SUPPORTED)
            DeleteFileW(LongPathForWindows(sDestintationFileName).wc_str());
        return (nRet);
    }

    wxLogMessage("Extracted a file.");

    CloseHandle(hFile);

    return (ERROR_SUCCESS);
}
