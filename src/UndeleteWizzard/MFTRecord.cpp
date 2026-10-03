// MFTRecord.cpp: implementation of the CMFTRecord class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "windows.h"
#include <wx/log.h>
#include "MFTRecord.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define DEFAULT_COPY_CHUNK_SIZE 1024*1024*1024

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFTRecord::CMFTRecord() {
    m_hDrive = 0;

    m_dwMaxMFTRecSize = 1023; // usual size
    m_pMFTRecord = 0;
    m_dwCurPos = 0;

    m_puchFileData = 0; // collected file data buffer
    m_llFileDataSz = 0; // file data size , ie. m_pchFileData buffer length

    memset(&m_attrStandard, 0, sizeof (ATTR_STANDARD));
    memset(&m_attrFilename, 0, sizeof (ATTR_FILENAME));

    m_bInUse = false;
    //populate recSystemInfo
    GetSystemInfo(&m_SystemInfo);
}

CMFTRecord::~CMFTRecord() {
    if (m_puchFileData)
        delete m_puchFileData;
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


/// puchMFTBuffer is the MFT record buffer itself (normally 1024 bytes)
//  dwLen is the MFT record buffer length
//  bExcludeData = if true the file data will not be extracted
//                 This is useful for only file browsing

int CMFTRecord::ExtractFile(BYTE *puchMFTBuffer, LONGLONG llLen, bool bExcludeData, HANDLE hFile) {
    if (m_dwMaxMFTRecSize > llLen)
        return ERROR_INVALID_PARAMETER;
    if (!puchMFTBuffer)
        return ERROR_INVALID_PARAMETER;

    NTFS_MFT_FILE ntfsMFT;
    NTFS_ATTRIBUTE ntfsAttr;
    ATTRIBUTE_RECORD_HEADER msattr;

    BYTE *puchTmp = 0;
    BYTE *uchTmpData = 0;
    LONGLONG llTmpDataLen;
    int nRet;

    m_pMFTRecord = puchMFTBuffer;
    m_dwCurPos = 0;

    /* --< EXTRACT ONLY ONE DATA STREAM >-- */
    int iDataProcessed = 0;


    if (m_puchFileData)
        delete m_puchFileData;
    m_puchFileData = 0;
    m_llFileDataSz = 0;

    // read the record header in MFT table
    memcpy(&ntfsMFT, &m_pMFTRecord[m_dwCurPos], sizeof (NTFS_MFT_FILE));

    if (memcmp(ntfsMFT.szSignature, "FILE", 4))
        return ERROR_INVALID_PARAMETER; // not the right signature

    m_bInUse = (ntfsMFT.wFlags & 0x01); //0x01  	Record is in use
    //0x02 	Record is a directory

    //m_dwCurPos = (ntfsMFT.wFixupOffset + ntfsMFT.wFixupSize*2); 
    m_dwCurPos = ntfsMFT.wAttribOffset;

    do { // extract the attribute header
        memcpy(&ntfsAttr, &m_pMFTRecord[m_dwCurPos], sizeof (NTFS_ATTRIBUTE));
        memcpy(&msattr, &m_pMFTRecord[m_dwCurPos], sizeof (ATTRIBUTE_RECORD_HEADER)); // ne koristimo u nasem resenju - Skinuto sa Microsofta

        switch (ntfsAttr.dwType) // extract the attribute data 
        {
                // here I haven' implemented the processing of all the attributes.
                //  I have implemented attributes necessary for file & file data extraction
            case 0://UNUSED
                break;

            case 0x10: //STANDARD_INFORMATION
                nRet = ExtractData(ntfsAttr, uchTmpData, llTmpDataLen, NULL);
                if (nRet)
                    return nRet;
                memcpy(&m_attrStandard, uchTmpData, sizeof (ATTR_STANDARD));

                delete uchTmpData;
                uchTmpData = 0;
                llTmpDataLen = 0;
                break;

            case 0x30: //FILE_NAME
                nRet = ExtractData(ntfsAttr, uchTmpData, llTmpDataLen, NULL);
                if (nRet)
                    return nRet;
                memcpy(&m_attrFilename, uchTmpData, llTmpDataLen);

                delete uchTmpData;
                uchTmpData = 0;
                llTmpDataLen = 0;

                break;

            case 0x40: //OBJECT_ID
                break;
            case 0x50: //SECURITY_DESCRIPTOR
                break;
            case 0x60: //VOLUME_NAME
                break;
            case 0x70: //VOLUME_INFORMATION
                break;
            case 0x80: //DATA
                if (!iDataProcessed) {
                    //this marks that there was allready one data stream and that if another should follow - do not process it                    
                    iDataProcessed = 1;


                    if (hFile) {//ovde se koristi extrakcija kada se koristi memory maped file, poenta je da samo samo taj mapirani fajl poturili kao bafer u vec postojeci algoritam
                        //ako zelis da razumes sta se tacno desava, proctaj kod koji ide ispod if(!bExcludeData)

                        nRet = ExtractData(ntfsAttr, uchTmpData, llTmpDataLen, hFile);
                        if (nRet)
                            return nRet;

                        m_llFileDataSz += llTmpDataLen;

                    } else if (!bExcludeData) { //ovde pocinje extrakcija kada se ne koristi memory mapped file.
                        nRet = ExtractData(ntfsAttr, uchTmpData, llTmpDataLen, NULL);
                        if (nRet)
                            return nRet;

                        if (!m_puchFileData) {
                            m_llFileDataSz = llTmpDataLen;
                            m_puchFileData = new BYTE[llTmpDataLen];

                            memcpy(m_puchFileData, uchTmpData, llTmpDataLen);
                        } else {
                            puchTmp = new BYTE[m_llFileDataSz + llTmpDataLen];
                            memcpy(puchTmp, m_puchFileData, m_llFileDataSz);
                            memcpy(puchTmp + m_llFileDataSz, uchTmpData, llTmpDataLen);

                            m_llFileDataSz += llTmpDataLen;
                            delete m_puchFileData;
                            m_puchFileData = puchTmp;
                        }

                        delete uchTmpData;
                        uchTmpData = 0;
                        llTmpDataLen = 0;
                    }
                }
                break;

            case 0x90: //INDEX_ROOT
            case 0xa0: //INDEX_ALLOCATION
                // todo: not implemented to read the index mapped records
                return ERROR_SUCCESS;
                continue;
                break;
            case 0xb0: //BITMAP
                break;
            case 0xc0: //REPARSE_POINT
                break;
            case 0xd0: //EA_INFORMATION
                break;
            case 0xe0: //EA
                break;
            case 0xf0: //PROPERTY_SET
                break;
            case 0x100: //LOGGED_UTILITY_STREAM
                break;
            case 0x1000: //FIRST_USER_DEFINED_ATTRIBUTE
                break;

            case 0xFFFFFFFF: // END 
                if (uchTmpData && !hFile)
                    delete uchTmpData;
                uchTmpData = 0;
                llTmpDataLen = 0;
                return ERROR_SUCCESS;

            default:
                break;
        };

        m_dwCurPos += ntfsAttr.dwFullLength; // go to the next location of attribute
    } while (ntfsAttr.dwFullLength && m_dwCurPos < 1024);

    if (uchTmpData)
        delete uchTmpData;
    uchTmpData = 0;
    llTmpDataLen = 0;
    return ERROR_SUCCESS;
}

// extract the attribute data from the MFT table
//   Data can be Resident & non-resident

int CMFTRecord::ExtractData(NTFS_ATTRIBUTE ntfsAttr, BYTE *&puchData, LONGLONG &llDataLen, HANDLE hFile) {
    DWORD dwCurPos = m_dwCurPos;
    HANDLE hMapObject;
    DWORD dwSizeHigh = 0, dwSizeLow = 0;
    DWORD dwErrorCode;
    LONGLONG llTempRemainingSize = 0; // used for iteration of a memory mapped object view
    DWORD dwNewChunkSize; // used for iteration of a memory mapped object view

    if (!ntfsAttr.uchNonResFlag) {// residence attribute, this always resides in the MFT table itself
        llDataLen = ntfsAttr.Attr.Resident.dwLength;

        if (hFile) { //koristi se mmap fajl, napravi baffer umesto puchData
            dwSizeLow = (llDataLen & 0x00000000FFFFFFFF);
            dwSizeHigh = ((llDataLen & 0xFFFFFFFF00000000) >> 32);
            hMapObject = CreateFileMapping(hFile, NULL, PAGE_READWRITE, dwSizeHigh, dwSizeLow, NULL);

            if (!hMapObject) {
                dwErrorCode = GetLastError();
                wxLogMessage("CreateFileMapping returned error code %d", dwErrorCode);
                return (dwErrorCode);
            }


            //this is very stupid, but function MapViewOfFile requires that the dwSizeHigh and dwSizeLow  represent a multiple of
            //system granularity. That granularity is in m_SystemInfo.dwAllocationGranularity
            //this is a problem when a file has two data streams.
            //In that case we open the buffer on a granularity multiple position and increment the pointer to reach the last position
            //we increment it by m_llFileDataSz, which is the last size of the file.

            LONGLONG llRemainder = m_llFileDataSz % m_SystemInfo.dwAllocationGranularity;
            LONGLONG llGranularityMultiple = m_llFileDataSz - llRemainder;

            dwSizeLow = (llGranularityMultiple & 0x00000000FFFFFFFF);
            dwSizeHigh = ((llGranularityMultiple & 0xFFFFFFFF00000000) >> 32);
            puchData = (BYTE *) MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, dwSizeHigh, dwSizeLow, 0);

            if (!puchData) {
                dwErrorCode = GetLastError();
                wxLogError("MapViewOfFile returned error code %d . Possibly not enough drive space", dwErrorCode); //sranje - ovde treba provera mesta na hardu i neko lepo preskakanje
                return (dwErrorCode);
            } else
                puchData += llRemainder; //this is the silly increment, to reach the end of the information that was written the last time. puchData is BYTE* so it should be OK
        } else
            puchData = new BYTE[ntfsAttr.Attr.Resident.dwLength];


        memcpy(puchData, &m_pMFTRecord[dwCurPos + ntfsAttr.Attr.Resident.wAttrOffset], llDataLen);
    } else {// non-residence attribute, this resides in the other part of the physical drive

        if (!ntfsAttr.Attr.NonResident.n64AllocSize) // i don't know Y, but fails when its zero
            ntfsAttr.Attr.NonResident.n64AllocSize = (ntfsAttr.Attr.NonResident.n64EndVCN - ntfsAttr.Attr.NonResident.n64StartVCN) + 1;

        // ATTR_STANDARD size may not be correct
        llDataLen = ntfsAttr.Attr.NonResident.n64RealSize;

        // allocate for reading data
        if (hFile) // ovaj uslov znaci da kad smo prosledili handle, onda se ide preko mmaped fajla. Ako nema handle-a onda se ide preko obicnih bafera.
            //mmaped file se podmece umesto puchData bafera.
        {
            // ovde ide velicina, pise 0x100000 a treba da se prevede dwDataLen + m_dwFileDataSz u hexa, tj u dva dword-a
            // prilikom kreiranja velicine FileMapping objekta nije bitna velicina, moze i vise gigabajta
            // problemi nastaju kad pravimo view tog objekta jer on mora da stane u adresni prostor procesa a to je kod 32bitnih sistema ispod 2GB (a ne 4GB??)
            dwSizeLow = ((llDataLen + m_llFileDataSz) & 0x00000000FFFFFFFF);
            dwSizeHigh = (((llDataLen + m_llFileDataSz) & 0xFFFFFFFF00000000) >> 32);
            hMapObject = CreateFileMapping(hFile, NULL, PAGE_READWRITE, dwSizeHigh, dwSizeLow, NULL);

            // ovde negde treba da se uradi racunanje offseta i racunaje velicine ;)
            //            velicina je dwDataLen + m_dwFileDataSz, zato sto je dwDataLen velicina novog atributa, a m_dwFileDataSz velicina svih prethodnih atributa koji su vec snimljeni
            //            a offset je m_dwFileDataSz

            if (!hMapObject) {
                dwErrorCode = GetLastError();
                wxLogError("CreateFileMapping returned error code %d .", dwErrorCode); //sranje - ovde treba provera mesta na hardu i neko lepo preskakanje
                return (dwErrorCode);
            }
            // ovde pravimo view FileMapping objekta. Realno ovde se mapira fajl na virtualni mmorijski adresni prostor.
            // zbog toga view ne moze da bude veliki. Postavicemo da view bude od offseta do kraja file mapping objekta, 
            // to se postavlja stavljanjem nule u poslednji argument.
            // ovde se radi ubacivanje offseta... hehe offset je m_dwFileDataSz - to su pretposlednja dva argumenta funkcije,
            // a poslednji broj je jednak ntfsAttr.Attr.NonResident.n64AllocSize, ili bolje 0, sto znaci do kraja map objekta
            // obzirom da view ne moze da bude veliki - ogranicicemo ga na 1GB i sad cemo da nabudzimo iteracije od po 1GB
            // da bi obradili ceo view u komadima od po 1GB

            //this is very stupid, but function MapViewOfFile requires that the dwSizeHigh and dwSizeLow represent a multiple of
            //system granularity. That granularity is in m_SystemInfo.dwAllocationGranularity
            //this is a problem when a file has two data streams.
            //In that case we open the buffer on a granularity multiple position and increment the pointer to reach the end of previous data stream
            //we increment it by llRemainder, which is the remainder when doing % division.
            LONGLONG llRemainder = m_llFileDataSz % m_SystemInfo.dwAllocationGranularity;
            LONGLONG llGranularityMultiple = m_llFileDataSz - llRemainder;

            dwSizeLow = (llGranularityMultiple & 0x00000000FFFFFFFF);
            dwSizeHigh = ((llGranularityMultiple & 0xFFFFFFFF00000000) >> 32);

            if (llDataLen > DEFAULT_COPY_CHUNK_SIZE) {
                llTempRemainingSize = llDataLen - DEFAULT_COPY_CHUNK_SIZE;
                dwNewChunkSize = DEFAULT_COPY_CHUNK_SIZE;
            } else {
                llTempRemainingSize = 0;
                dwNewChunkSize = llDataLen;
            }

            puchData = (BYTE *) MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, dwSizeHigh, dwSizeLow, dwNewChunkSize);

            if (!puchData) {
                dwErrorCode = GetLastError();
                wxLogMessage("MapViewOfFile returned error code %d", dwErrorCode);
                return (dwErrorCode);
            } else
                puchData += llRemainder; //this is the silly increment, to reach the end of the information that was written the last time. puchData is BYTE* so it should be OK

        } else
            puchData = new BYTE[ntfsAttr.Attr.NonResident.n64AllocSize]; //ovde ne koristimo mmap fajlove

        BYTE chLenOffSz; // length & offset sizes
        BYTE chLenSz; // length size
        BYTE chOffsetSz; // offset size
        LONGLONG n64Len, n64Offset; // the actual lenght & offset
        LONGLONG n64LCN = 0; // the pointer pointing the actual data on a physical disk
        BYTE *pTmpBuff = puchData;
        int nRet;

        dwCurPos += ntfsAttr.Attr.NonResident.wDatarunOffset;

        for (;;) {
            ///// read the length of LCN/VCN and length ///////////////////////
            chLenOffSz = 0;

            memcpy(&chLenOffSz, &m_pMFTRecord[dwCurPos], sizeof (BYTE));

            dwCurPos += sizeof (BYTE);

            if (!chLenOffSz)
                break;

            chLenSz = chLenOffSz & 0x0F;
            chOffsetSz = (chLenOffSz & 0xF0) >> 4;

            ///// read the data length ////////////////////////////////////////

            n64Len = 0;

            memcpy(&n64Len, &m_pMFTRecord[dwCurPos], chLenSz);

            dwCurPos += chLenSz;

            ///// read the LCN/VCN offset //////////////////////////////////////

            n64Offset = 0;

            memcpy(&n64Offset, &m_pMFTRecord[dwCurPos], chOffsetSz);

            dwCurPos += chOffsetSz;

            ////// if the last bit of n64Offset is 1 then its -ve so u got to make it -ve /////
            if ((((char*) &n64Offset)[chOffsetSz - 1])&0x80)
                for (int i = sizeof (LONGLONG) - 1; i > (chOffsetSz - 1); i--)
                    ((char*) &n64Offset)[i] = 0xff;

            n64LCN += n64Offset;

            n64Len *= m_dwBytesPerCluster; // hmpf  - ovo je malo zeznuto - ne znam da li da koristim u vrednost ili vrednost procitanu iz atributa.
            ///// read the actual data /////////////////////////////////////////
            /// since the data is available out side the MFT table, physical drive should be accessed

            if (hFile) {
                while (dwNewChunkSize > 0) {
                    //nRet = ReadRaw(n64LCN, pTmpBuff, (DWORD&) n64Len);
                    nRet = ReadRaw(n64LCN, pTmpBuff, dwNewChunkSize);
                    if (nRet) {
                        dwErrorCode = GetLastError();
                        wxLogError("ReadRaw returned error code %d .", dwErrorCode); //sranje - ovde treba provera mesta na hardu i neko lepo preskakanje
                        return (dwErrorCode);
                    }

                    m_llFileDataSz += dwNewChunkSize;
                    n64LCN += (dwNewChunkSize / m_dwBytesPerCluster);
                    dwSizeLow = ((m_llFileDataSz) & 0x00000000FFFFFFFF);
                    dwSizeHigh = (((m_llFileDataSz) & 0xFFFFFFFF00000000) >> 32);

                    if (llTempRemainingSize > DEFAULT_COPY_CHUNK_SIZE) {
                        llTempRemainingSize -= DEFAULT_COPY_CHUNK_SIZE;
                        dwNewChunkSize = DEFAULT_COPY_CHUNK_SIZE;
                    } else {
                        dwNewChunkSize = llTempRemainingSize;
                        llTempRemainingSize = 0;
                    }

                    UnmapViewOfFile(pTmpBuff);
                    if (dwNewChunkSize) {
                        pTmpBuff = (BYTE *) MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, dwSizeHigh, dwSizeLow, dwNewChunkSize);
                        if (!pTmpBuff) {
                            dwErrorCode = GetLastError();
                            wxLogError("MapViewOfFile returned error code %d .", dwErrorCode); //sranje - ovde treba provera mesta na hardu i neko lepo preskakanje
                            return (dwErrorCode);
                        }
                    }
                }
            } else {
                nRet = ReadRaw(n64LCN, pTmpBuff, (DWORD&) n64Len);
                if (nRet) {
                    dwErrorCode = GetLastError();
                    wxLogError("ReadRaw returned error code %d .", dwErrorCode); //sranje - ovde treba provera mesta na hardu i neko lepo preskakanje
                    return (dwErrorCode);
                }
            }
            pTmpBuff += n64Len;
        }
    }

    if (hFile) {
        UnmapViewOfFile(puchData);
        CloseHandle(hMapObject);
        wxLogMessage("Un-mapping View and Closing Handle");
    }
    return ERROR_SUCCESS;
}

// read the data from the physical drive

int CMFTRecord::ReadRaw(LONGLONG n64LCN, BYTE *chData, DWORD & dwLen) {
    int nRet;
    DWORD dwRet;
    DWORD dwErrorCode;

    LARGE_INTEGER n64Pos;

    n64Pos.QuadPart = (n64LCN) * m_dwBytesPerCluster;
    n64Pos.QuadPart += m_n64StartPos;

    //   data is available in the relative sector from the begining od the drive	
    //    so point that data
    dwRet = SetFilePointer(m_hDrive, n64Pos.LowPart, &n64Pos.HighPart, FILE_BEGIN);
    if (INVALID_SET_FILE_POINTER == dwRet) {
        dwErrorCode = GetLastError();
        wxLogError("SetFilePointer returned error code %d .", dwErrorCode); //sranje - ovde treba provera mesta na hardu i neko lepo preskakanje
        return (dwErrorCode);
    }

    BYTE *pTmp = chData;
    DWORD dwBytesRead = 0;
    DWORD dwBytes = 0;
    DWORD dwTotRead = 0;

    while (dwTotRead < dwLen) {
        // v r reading a cluster at a time
        dwBytesRead = m_dwBytesPerCluster;

        // this can not read partial sectors
        nRet = ReadFile(m_hDrive, pTmp, dwBytesRead, &dwBytes, NULL);
        if (!nRet) {
            dwErrorCode = GetLastError();
            wxLogError("ReadRaw returned error code %d .", dwErrorCode); //sranje - ovde treba provera mesta na hardu i neko lepo preskakanje
            return (dwErrorCode);
        }

        dwTotRead += dwBytes;
        pTmp += dwBytes;
    }

    dwLen = dwTotRead;

    return ERROR_SUCCESS;
}



