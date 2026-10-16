// MFTRecord.cpp: implementation of the CMFTRecord class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "windows.h"
#include <new>
#include <vector>
#include <wx/log.h>
#include "MFTRecord.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// size of the bounce buffer used to copy file data from the volume (rounded down to a cluster multiple)
#define DEFAULT_COPY_CHUNK_SIZE (1024*1024)

// FILE_NAME namespace 2 is the short DOS (8.3) name, e.g. "PROGRA~1"
#define FILE_NAME_NAMESPACE_DOS 2

// attribute header flags
#define ATTR_FLAG_COMPRESSED 0x0001

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFTRecord::CMFTRecord() {
    m_hDrive = 0;

    m_dwMaxMFTRecSize = 1024; // usual size
    m_pMFTRecord = 0;
    m_dwCurPos = 0;
    m_dwBytesPerCluster = 0;
    m_n64StartPos = 0;

    m_puchFileData = 0; // collected file data buffer
    m_llFileDataSz = 0; // file data size , ie. m_pchFileData buffer length
    m_llDataSize = 0;

    memset(&m_attrStandard, 0, sizeof (ATTR_STANDARD));
    memset(&m_attrFilename, 0, sizeof (ATTR_FILENAME));

    m_bInUse = false;
}

CMFTRecord::~CMFTRecord() {
    delete[] m_puchFileData;
    m_puchFileData = 0;
    m_llFileDataSz = 0;
}

// set the drive handle

void CMFTRecord::SetDriveHandle(HANDLE hDrive) {
    m_hDrive = hDrive;
}

// set the detail
//  n64StartPos is the byte from the starting of the physical disk
//  dwRecSize is the record size in the MFT table
//  dwBytesPerCluster is the bytes per cluster

int CMFTRecord::SetRecordInfo(LONGLONG n64StartPos, DWORD dwRecSize, DWORD dwBytesPerCluster) {
    if (!dwRecSize)
        return ERROR_INVALID_PARAMETER;

    if (dwRecSize % 2)
        return ERROR_INVALID_PARAMETER;

    if (!dwBytesPerCluster)
        return ERROR_INVALID_PARAMETER;

    if (dwBytesPerCluster % 2)
        return ERROR_INVALID_PARAMETER;

    m_dwMaxMFTRecSize = dwRecSize;
    m_dwBytesPerCluster = dwBytesPerCluster;
    m_n64StartPos = n64StartPos;
    return ERROR_SUCCESS;
}

// NTFS protects multi-sector records against torn writes: before writing a record it saves the last
// WORD of every 512-byte stride into the "update sequence array" (fixup array) and replaces it with
// the update sequence number. Those WORDs have to be put back before the record is parsed, otherwise
// any attribute (file name, data runs...) that crosses byte 510, 1022... reads corrupted data.

int CMFTRecord::ApplyFixups(BYTE *puchRecord, DWORD dwRecSize) {
    NTFS_MFT_FILE *pHeader = (NTFS_MFT_FILE *) puchRecord;
    DWORD dwOffset = pHeader->wFixupOffset;
    DWORD dwCount = pHeader->wFixupSize; // update sequence number + one entry per stride

    if (dwCount < 2 || dwOffset + dwCount * sizeof (WORD) > dwRecSize)
        return ERROR_FILE_CORRUPT;

    DWORD dwStride = dwRecSize / (dwCount - 1);
    if (dwStride < sizeof (WORD) || dwStride * (dwCount - 1) != dwRecSize)
        return ERROR_FILE_CORRUPT;

    WORD *pwFixups = (WORD *) (puchRecord + dwOffset);
    WORD wSequenceNumber = pwFixups[0];

    for (DWORD i = 1; i < dwCount; i++) {
        WORD *pwSectorEnd = (WORD *) (puchRecord + i * dwStride - sizeof (WORD));
        if (*pwSectorEnd != wSequenceNumber)
            return ERROR_FILE_CORRUPT; // torn write - the record is not consistent
        *pwSectorEnd = pwFixups[i];
    }
    return ERROR_SUCCESS;
}

/// puchMFTBuffer is the MFT record buffer itself (normally 1024 bytes). Fixups are applied to it in place.
//  dwLen is the MFT record buffer length
//  bExcludeData = if true the file data will not be extracted
//                 This is useful for only file browsing
//  hFile = if set, the file data is written to this file instead of being collected in m_puchFileData

int CMFTRecord::ExtractFile(BYTE *puchMFTBuffer, LONGLONG llLen, bool bExcludeData, HANDLE hFile) {
    if (!puchMFTBuffer)
        return ERROR_INVALID_PARAMETER;
    if (llLen < m_dwMaxMFTRecSize || m_dwMaxMFTRecSize < sizeof (NTFS_MFT_FILE))
        return ERROR_INVALID_PARAMETER;

    DWORD dwRecSize = m_dwMaxMFTRecSize;
    int nRet;

    m_pMFTRecord = puchMFTBuffer;
    m_dwCurPos = 0;

    delete[] m_puchFileData;
    m_puchFileData = 0;
    m_llFileDataSz = 0;
    m_vDataRuns.clear();
    m_llDataSize = 0;
    memset(&m_attrStandard, 0, sizeof (ATTR_STANDARD));
    memset(&m_attrFilename, 0, sizeof (ATTR_FILENAME));
    bool bHaveFilename = false;

    // read the record header in MFT table
    NTFS_MFT_FILE ntfsMFT;
    memcpy(&ntfsMFT, m_pMFTRecord, sizeof (NTFS_MFT_FILE));

    if (memcmp(ntfsMFT.szSignature, "FILE", 4))
        return ERROR_FILE_INVALID; // not the right signature - unused or damaged record

    nRet = ApplyFixups(m_pMFTRecord, dwRecSize);
    if (nRet)
        return nRet;

    m_bInUse = (ntfsMFT.wFlags & 0x01); //0x01  	Record is in use
    //0x02 	Record is a directory

    // only walk the part of the record that is actually used
    DWORD dwLimit = dwRecSize;
    if (ntfsMFT.dwRecLength && ntfsMFT.dwRecLength < dwLimit)
        dwLimit = ntfsMFT.dwRecLength;

    /* --< EXTRACT ONLY ONE DATA STREAM >-- */
    bool bDataProcessed = false;

    m_dwCurPos = ntfsMFT.wAttribOffset;

    while (m_dwCurPos + 2 * sizeof (DWORD) <= dwLimit) { // extract the attribute header
        const BYTE *puchAttr = &m_pMFTRecord[m_dwCurPos];
        DWORD dwType = *(const DWORD *) puchAttr;

        if (dwType == 0xFFFFFFFF) // END
            return ERROR_SUCCESS;

        DWORD dwFullLength = *(const DWORD *) (puchAttr + sizeof (DWORD));
        if (dwFullLength < 16 || dwFullLength > dwLimit - m_dwCurPos) {
            wxLogMessage("MFT record has a damaged attribute (type 0x%x, length %u).", dwType, dwFullLength);
            return ERROR_FILE_CORRUPT;
        }

        // the header struct is larger than a short resident attribute - never copy past the attribute
        NTFS_ATTRIBUTE ntfsAttr;
        memset(&ntfsAttr, 0, sizeof (NTFS_ATTRIBUTE));
        memcpy(&ntfsAttr, puchAttr, dwFullLength < sizeof (NTFS_ATTRIBUTE) ? dwFullLength : sizeof (NTFS_ATTRIBUTE));

        // resident value of this attribute, if it has one that fits inside the attribute
        const BYTE *puchValue = 0;
        DWORD dwValueLen = 0;
        if (!ntfsAttr.uchNonResFlag && dwFullLength >= 24
                && (DWORD) ntfsAttr.Attr.Resident.wAttrOffset + ntfsAttr.Attr.Resident.dwLength <= dwFullLength) {
            puchValue = puchAttr + ntfsAttr.Attr.Resident.wAttrOffset;
            dwValueLen = ntfsAttr.Attr.Resident.dwLength;
        }

        switch (dwType) // extract the attribute data
        {
                // here I haven' implemented the processing of all the attributes.
                //  I have implemented attributes necessary for file & file data extraction
            case 0x10: //STANDARD_INFORMATION
                if (puchValue)
                    memcpy(&m_attrStandard, puchValue, dwValueLen < sizeof (ATTR_STANDARD) ? dwValueLen : sizeof (ATTR_STANDARD));
                break;

            case 0x30: //FILE_NAME
            {
                // a file usually has two names - the long one and the short DOS (8.3) one. Keep the long one.
                DWORD dwNameOffset = (DWORD) ((BYTE *) m_attrFilename.wFilename - (BYTE *) & m_attrFilename);
                if (!puchValue || dwValueLen < dwNameOffset)
                    break;

                ATTR_FILENAME *pName = (ATTR_FILENAME *) puchValue;
                if (bHaveFilename && pName->chFileNameType == FILE_NAME_NAMESPACE_DOS)
                    break;

                DWORD dwNameLen = pName->chFileNameLength;
                if (dwNameOffset + dwNameLen * sizeof (WORD) > dwValueLen)
                    break; // name runs past the attribute

                memset(&m_attrFilename, 0, sizeof (ATTR_FILENAME));
                memcpy(&m_attrFilename, puchValue, dwNameOffset + dwNameLen * sizeof (WORD));
                m_attrFilename.wFilename[dwNameLen] = 0; // the name on disk is not zero terminated
                bHaveFilename = true;
                break;
            }

            case 0x80: //DATA
                // only the unnamed (main) data stream - named streams are alternate data streams
                if (!bDataProcessed && !ntfsAttr.uchNameLength) {
                    //this marks that there was allready one data stream and that if another should follow - do not process it
                    bDataProcessed = true;

                    // remember where the data is, without reading it (used to read the $MFT itself in pieces)
                    if (!ntfsAttr.uchNonResFlag) {
                        m_llDataSize = ntfsAttr.Attr.Resident.dwLength;
                    } else if (dwFullLength >= 64 && ntfsAttr.Attr.NonResident.n64StartVCN == 0
                            && ntfsAttr.Attr.NonResident.wDatarunOffset < dwFullLength) {
                        m_llDataSize = ntfsAttr.Attr.NonResident.n64RealSize;
                        DWORD dwRunsOffset = ntfsAttr.Attr.NonResident.wDatarunOffset;
                        if (ParseDataRuns(puchAttr + dwRunsOffset, dwFullLength - dwRunsOffset, m_vDataRuns))
                            m_vDataRuns.clear();
                    }

                    if (!hFile && bExcludeData)
                        break;

                    BYTE *puchData = 0;
                    LONGLONG llDataLen = 0;
                    nRet = ExtractData(puchAttr, ntfsAttr, puchData, llDataLen, hFile);
                    if (nRet)
                        return nRet;

                    m_puchFileData = puchData; // stays 0 when the data went to hFile
                    m_llFileDataSz = llDataLen;
                }
                break;

            case 0x90: //INDEX_ROOT
            case 0xa0: //INDEX_ALLOCATION
                // todo: not implemented to read the index mapped records
                return ERROR_SUCCESS;

            default:
                break;
        };

        m_dwCurPos += dwFullLength; // go to the next location of attribute
    }

    return ERROR_SUCCESS;
}

// decode the data run list (mapping pairs) of a non-resident attribute.
// Each run is: header byte (low nibble = size of length field, high nibble = size of offset field),
// the length in clusters, then the LCN offset relative to the previous run (signed). No offset = sparse.

int CMFTRecord::ParseDataRuns(const BYTE *puchRuns, DWORD dwRunsLen, std::vector<NTFS_DATA_RUN> &vRuns) {
    DWORD dwPos = 0;
    LONGLONG n64LCN = 0;

    vRuns.clear();

    while (dwPos < dwRunsLen) {
        BYTE chLenOffSz = puchRuns[dwPos++];
        if (!chLenOffSz)
            return ERROR_SUCCESS; // end of the run list

        DWORD chLenSz = chLenOffSz & 0x0F;
        DWORD chOffsetSz = (chLenOffSz & 0xF0) >> 4;

        if (!chLenSz || chLenSz > 8 || chOffsetSz > 8 || dwPos + chLenSz + chOffsetSz > dwRunsLen)
            return ERROR_FILE_CORRUPT;

        ///// read the data length ////////////////////////////////////////
        LONGLONG n64Len = 0;
        memcpy(&n64Len, &puchRuns[dwPos], chLenSz);
        dwPos += chLenSz;
        if (n64Len <= 0)
            return ERROR_FILE_CORRUPT;

        NTFS_DATA_RUN run;
        run.llClusters = n64Len;
        run.bSparse = (chOffsetSz == 0);
        run.llLCN = 0;

        if (!run.bSparse) {
            ///// read the LCN offset - it is signed, relative to the previous run //////
            LONGLONG n64Offset = 0;
            memcpy(&n64Offset, &puchRuns[dwPos], chOffsetSz);
            if (puchRuns[dwPos + chOffsetSz - 1] & 0x80) // negative - sign extend
                for (DWORD i = chOffsetSz; i < sizeof (LONGLONG); i++)
                    ((BYTE *) & n64Offset)[i] = 0xFF;
            dwPos += chOffsetSz;

            n64LCN += n64Offset;
            if (n64LCN < 0)
                return ERROR_FILE_CORRUPT;
            run.llLCN = n64LCN;
        }

        vRuns.push_back(run);
    }

    return ERROR_FILE_CORRUPT; // the run list must end with a zero byte inside the attribute
}

// extract the data of a DATA attribute (or of the $MFT itself)
//   Data can be resident (stored in the MFT record) or non-resident (stored in data runs on the volume).
//   If hFile is set the data is written to it, otherwise a buffer is allocated into puchData (caller frees with delete[]).

int CMFTRecord::ExtractData(const BYTE *puchAttr, const NTFS_ATTRIBUTE &ntfsAttr, BYTE *&puchData, LONGLONG &llDataLen, HANDLE hFile) {
    DWORD dwFullLength = ntfsAttr.dwFullLength;
    DWORD dwWritten;

    puchData = 0;
    llDataLen = 0;

    if (!ntfsAttr.uchNonResFlag) {// residence attribute, this always resides in the MFT table itself
        if (dwFullLength < 24 || (DWORD) ntfsAttr.Attr.Resident.wAttrOffset + ntfsAttr.Attr.Resident.dwLength > dwFullLength)
            return ERROR_FILE_CORRUPT;

        const BYTE *puchValue = puchAttr + ntfsAttr.Attr.Resident.wAttrOffset;
        llDataLen = ntfsAttr.Attr.Resident.dwLength;

        if (hFile) {
            if (llDataLen && (!WriteFile(hFile, puchValue, (DWORD) llDataLen, &dwWritten, NULL) || dwWritten != llDataLen)) {
                DWORD dwErrorCode = GetLastError();
                wxLogError("WriteFile returned error code %d . Possibly not enough drive space", dwErrorCode);
                return dwErrorCode ? dwErrorCode : ERROR_WRITE_FAULT;
            }
        } else {
            puchData = new (std::nothrow) BYTE[llDataLen ? llDataLen : 1];
            if (!puchData)
                return ERROR_NOT_ENOUGH_MEMORY;
            memcpy(puchData, puchValue, llDataLen);
        }
        return ERROR_SUCCESS;
    }

    // non-residence attribute, this resides in the other part of the physical drive
    if (dwFullLength < 64)
        return ERROR_FILE_CORRUPT;

    if (ntfsAttr.wFlags & ATTR_FLAG_COMPRESSED) {
        // the clusters hold LZNT1 compressed data - copying them raw would produce a garbage file
        wxLogMessage("File data is NTFS-compressed - recovering compressed files is not supported.");
        return ERROR_NOT_SUPPORTED;
    }

    if (ntfsAttr.Attr.NonResident.n64StartVCN != 0) {
        // the first part of the data is described in another MFT record (attribute list) - not supported
        wxLogMessage("File data starts in another MFT record - not supported.");
        return ERROR_NOT_SUPPORTED;
    }

    std::vector<NTFS_DATA_RUN> vRuns;
    DWORD dwRunsOffset = ntfsAttr.Attr.NonResident.wDatarunOffset;
    if (dwRunsOffset >= dwFullLength)
        return ERROR_FILE_CORRUPT;

    int nRet = ParseDataRuns(puchAttr + dwRunsOffset, dwFullLength - dwRunsOffset, vRuns);
    if (nRet) {
        wxLogMessage("MFT record has a damaged data run list.");
        return nRet;
    }

    // ATTR_STANDARD size may not be correct - the real size of the data is in the DATA attribute
    LONGLONG llRealSize = ntfsAttr.Attr.NonResident.n64RealSize;

    LONGLONG llClusters = 0;
    for (size_t i = 0; i < vRuns.size(); i++)
        llClusters += vRuns[i].llClusters;

    if (llRealSize < 0 || llRealSize > llClusters * m_dwBytesPerCluster) {
        // more data than clusters - the data runs continue in another MFT record (attribute list) or the record is damaged
        wxLogMessage("File size (%" wxLongLongFmtSpec "d) is larger than its clusters (%" wxLongLongFmtSpec "d) - not supported or damaged record.",
                llRealSize, llClusters * m_dwBytesPerCluster);
        return ERROR_NOT_SUPPORTED;
    }

    if (!hFile) {
        if ((unsigned long long) llRealSize > 0x7FFFFFFF)
            return ERROR_NOT_ENOUGH_MEMORY;
        puchData = new (std::nothrow) BYTE[llRealSize ? (size_t) llRealSize : 1];
        if (!puchData)
            return ERROR_NOT_ENOUGH_MEMORY;
    }

    // bounce buffer - always a whole number of clusters, because the volume can only be read in whole sectors
    DWORD dwChunkSize = (DEFAULT_COPY_CHUNK_SIZE / m_dwBytesPerCluster) * m_dwBytesPerCluster;
    if (!dwChunkSize)
        dwChunkSize = m_dwBytesPerCluster;

    std::vector<BYTE> vBuffer;
    try {
        vBuffer.resize(dwChunkSize);
    } catch (std::bad_alloc &) {
        delete[] puchData;
        puchData = 0;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    LONGLONG llRemaining = llRealSize;
    BYTE *puchOut = puchData;

    // walk the fragments in order - every fragment is somewhere else on the volume
    for (size_t i = 0; i < vRuns.size() && llRemaining > 0; i++) {
        const NTFS_DATA_RUN &run = vRuns[i];
        LONGLONG llRunBytes = run.llClusters * m_dwBytesPerCluster;
        LONGLONG llRunPos = 0;

        while (llRunPos < llRunBytes && llRemaining > 0) {
            DWORD dwChunk = (llRunBytes - llRunPos < dwChunkSize) ? (DWORD) (llRunBytes - llRunPos) : dwChunkSize;
            // the last chunk of the file only needs part of its last cluster
            DWORD dwUseful = (llRemaining < dwChunk) ? (DWORD) llRemaining : dwChunk;

            if (run.bSparse) {
                memset(&vBuffer[0], 0, dwUseful);
            } else {
                DWORD dwRead = ((dwUseful + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster) * m_dwBytesPerCluster;
                nRet = ReadRaw(run.llLCN + llRunPos / m_dwBytesPerCluster, &vBuffer[0], dwRead);
                if (nRet) {
                    delete[] puchData;
                    puchData = 0;
                    return nRet;
                }
            }

            if (hFile) {
                if (!WriteFile(hFile, &vBuffer[0], dwUseful, &dwWritten, NULL) || dwWritten != dwUseful) {
                    DWORD dwErrorCode = GetLastError();
                    wxLogError("WriteFile returned error code %d . Possibly not enough drive space", dwErrorCode);
                    return dwErrorCode ? dwErrorCode : ERROR_WRITE_FAULT;
                }
            } else {
                memcpy(puchOut, &vBuffer[0], dwUseful);
                puchOut += dwUseful;
            }

            llRunPos += dwChunk;
            llRemaining -= dwUseful;
        }
    }

    llDataLen = llRealSize;
    return ERROR_SUCCESS;
}

// read dwLen bytes (a whole number of clusters) starting at cluster n64LCN of the volume

int CMFTRecord::ReadRaw(LONGLONG n64LCN, BYTE *chData, DWORD dwLen) {
    DWORD dwErrorCode;
    LARGE_INTEGER n64Pos;

    n64Pos.QuadPart = (n64LCN) * m_dwBytesPerCluster;
    n64Pos.QuadPart += m_n64StartPos;

    //   data is available in the relative sector from the begining od the drive
    //    so point that data
    if (!SetFilePointerEx(m_hDrive, n64Pos, NULL, FILE_BEGIN)) {
        dwErrorCode = GetLastError();
        wxLogError("SetFilePointer returned error code %d .", dwErrorCode);
        return (dwErrorCode);
    }

    DWORD dwTotRead = 0;
    while (dwTotRead < dwLen) {
        DWORD dwBytes = 0;
        if (!ReadFile(m_hDrive, chData + dwTotRead, dwLen - dwTotRead, &dwBytes, NULL)) {
            dwErrorCode = GetLastError();
            wxLogError("ReadRaw returned error code %d .", dwErrorCode);
            return (dwErrorCode);
        }
        if (!dwBytes) {
            wxLogError("ReadRaw reached the end of the volume at cluster %" wxLongLongFmtSpec "d .", n64LCN);
            return ERROR_HANDLE_EOF;
        }
        dwTotRead += dwBytes;
    }

    return ERROR_SUCCESS;
}
