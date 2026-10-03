#include <windows.h>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/wizard.h>
#include <wx/event.h>
#include <wx/progdlg.h>

#include "NTFSDrive.h"
#include "Page_SelectSourceDrivePage.h"
#include "MyWizzard.h"
#include "languages/LanguageEnums.h"

//Various pictures
#include "resources/drive-harddisk-5.xpm"
#include "resources/drive-optical-3.xpm"
#include "resources/drive-removable-media-3.xpm"
#include "resources/media-flash-2.xpm"
#include "resources/SidePicture1.xpm"
//#include "desktopIcon256x256.xpm"
#include "resources/desktopIcon64x64.xpm"

#include <shlobj.h>

//#define NTFS_ONLY 0

bool wxSelectSourceDrivePage::InvalidDiskAccessRights(wxString *ptrSelectedDesk) {

    NTFS_PART_BOOT_SEC ntfsBS;
    LARGE_INTEGER n84StartPos;

    char tempDrive[10];
    memset(tempDrive, '\0', 10 * sizeof (char));
    strncpy(tempDrive, "\\\\.\\", 7);
    strncat(tempDrive, ptrSelectedDesk->mb_str(), 2);

    wchar_t rDrive[10];
    memset(rDrive, '\0', 10 * sizeof (wchar_t));

    mbstowcs(rDrive, tempDrive, 9);

    memset(&n84StartPos, '\0', sizeof (n84StartPos));
    memset(&ntfsBS, '\0', sizeof (ntfsBS));
    //        DWORD lpSectorsPerCluster, lpBytesPerSector, lpNumberOfFreeClusters, lpTotalNumberOfClusters;

    HANDLE hDrive = CreateFile(rDrive, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (hDrive == INVALID_HANDLE_VALUE) {
        CloseHandle(hDrive);
        return (true);
    }

    CloseHandle(hDrive);
    return (false);
}

wxSelectSourceDrivePage::wxSelectSourceDrivePage(wxWizard *parent) : wxWizardPageSimple(parent) { // hehe - very big constructor - needs to be split in several functions
    m_bitmap = wxBitmap(SidePicture1_xpm);

    this->SetOwnBackgroundColour(wxColour(210, 222, 255));
    this->SetOwnForegroundColour(wxColour(210, 222, 255));
    this->SetForegroundColour(wxColour(210, 222, 255));
    this->SetBackgroundColour(wxColour(210, 222, 255));
    
    ILanguage *languagePack = ((MyWizard *) GetParent())->GetLanguagePack();

    wxStaticText *wxSelectDriveMessage = new wxStaticText(this, -1,
            languagePack->getWXText(SELECT_SOURCE_DRIVE_PAGE, PLEASE_SELECT_SOURCE));

    wxSelectDriveMessage->SetFont(wxFont(11, wxSWISS, wxNORMAL, wxNORMAL));

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(wxSelectDriveMessage, 0, wxALL, 0);


    m_imageListNormal = new wxImageList(32, 32, true);
    m_imageListSmall = new wxImageList(16, 16, true); // videcu da li cu ovu uopste koristiti

    int iDriveHardDiskImage = m_imageListNormal->Add(wxIcon(drive_harddisk__));
    int iDriveOpticalImage = m_imageListNormal->Add(wxIcon(drive_optical__));
    int iDriveRemovableMediaImage = m_imageListNormal->Add(wxIcon(drive_removable_media__));
    int iDriveMediaFlashImage = m_imageListNormal->Add(wxIcon(media_flash__));
    int iDesktopImage = m_imageListNormal->Add(wxIcon(desktopIcon64x64_xpm));


    m_DriveListCtrlPtr = new wxListCtrl(this, LIST_CTRL_SOURCE_DRIVES,
            wxDefaultPosition, wxDefaultSize,
            wxBORDER_THEME | wxLC_ICON | wxLC_AUTOARRANGE | wxLC_SINGLE_SEL);

    m_DriveListCtrlPtr->ClearAll();

    m_DriveListCtrlPtr->Show(false);

    m_DriveListCtrlPtr->SetImageList(m_imageListNormal, wxIMAGE_LIST_NORMAL);

    /* odavde krene petlja koja cita drajvove i trpa ih u listu*/
    wxListItem *ptrListItem = new wxListItem();
    int iListIndex = 0; // used to assign an index to all items

    wchar_t szDriveInformation[1024];
    memset(szDriveInformation, '\0', 1024 * sizeof (wchar_t));

    wchar_t tempBuff[64];
    memset(tempBuff, '\0', 64 * sizeof (wchar_t));

    GetLogicalDriveStrings(1024, szDriveInformation);
    bool bOK = true;

    wchar_t * szDriveLetters = szDriveInformation;

    LPCTSTR szHD;
    wchar_t szFileSys[255], szVolNameBuff[255]; // zasto je ovo UCHAR - nemam pojma - Zoran ga tako programirao i ja ga samo copy/paste
    DWORD dwSerial, dwMFL, dwSysFlags;
    BOOL bSuccess;
    wxString strLabel, strDriveSize;

    strDriveSize.Printf("Size Unknown");// TODO ovo treba prebaciti na odabrani jezik. Treba mi posebna funkcija koja vrata obican tekst, a ne wxT

    wchar_t wsNTFS[4 + 1] = L"NTFS";

    unsigned __int64 i64FreeBytesToCaller,
            i64TotalBytes,
            i64FreeBytes;

    // //postavljanje desktop ikonice
    {
        memset(szFileSys, '\0', 255 * sizeof (wchar_t));
        memset(szVolNameBuff, '\0', 255 * sizeof (wchar_t));
        memset(&dwSerial, '\0', sizeof (DWORD));
        memset(&dwMFL, '\0', sizeof (DWORD));
        memset(&dwSysFlags, '\0', sizeof (DWORD));

        TCHAR lPath[2048];
        memset(lPath, '\0', 2048 * sizeof (TCHAR));

        if (TRUE == SHGetSpecialFolderPath(
                (HWND)this,
                lPath,
                CSIDL_DESKTOP,
                0)
                ) {

        }

        memset(lPath + 3, '\0', (2048 - 3) * sizeof (TCHAR));
        szHD = lPath;

        bSuccess = GetVolumeInformation(szHD, szVolNameBuff,
                255, &dwSerial, &dwMFL, &dwSysFlags,
                szFileSys, 255);

        // debugging purpose
        //        if (!bSuccess) {
        //            DWORD dwErrorCode;
        //            dwErrorCode = GetLastError();
            //            wxLogMessage("CreateFileMapping returned error code %d", dwErrorCode);            
//        }

        //        bSuccess = bSuccess && GetDiskFreeSpaceEx(szHD,
        //                (PULARGE_INTEGER) & i64FreeBytesToCaller,
        //                (PULARGE_INTEGER) & i64TotalBytes,
        //                (PULARGE_INTEGER) & i64FreeBytes);

        /*    printf("Available space to caller = %I64u MB\n",
                      i64FreeBytesToCaller / (1024 * 1024));
              printf("Total space               = %I64u MB\n",
                      i64TotalBytes / (1024 * 1024));
              printf("Free space on drive       = %I64u MB\n",
                      i64FreeBytes / (1024 * 1024));
          }*/

        //        {
        //            float dSize = i64TotalBytes / (1024.00 * 1024.00 * 1024.00);
        //            if (bSuccess) {
        //                if (dSize < 1) {
        //                    dSize = i64TotalBytes / (1024.00 * 1024.00);
        //                    strDriveSize.Printf("%.2f MB", dSize);
        //                } else
        //                    strDriveSize.Printf("%.2f GB", dSize);
        //            } else
        //                break;
        //        }
#ifdef NTFS_ONLY 
        if (!wcscmp(szFileSys, wsNTFS)) 
#endif
        {
            strLabel.Printf("Desktop"); //TODO uraditi lokalizaciju
            ptrListItem->SetText(strLabel);
            ptrListItem->SetData((void*) (new wxString(szHD))); //ovde stoji bas ime drajva, na primer "D:\\"
            ptrListItem->SetImage(iDesktopImage);
            ptrListItem->SetId(iListIndex++);
            m_DriveListCtrlPtr->InsertItem(*ptrListItem);
        }
        //kraj postavljanja desktop ikonice
    }
    while (szDriveLetters && bOK) { //reading the drive information and populating the list

        /* Clear all temp buffers */
        memset(szFileSys, '\0', 255 * sizeof (wchar_t));
        memset(szVolNameBuff, '\0', 255 * sizeof (wchar_t));
        memset(&dwSerial, '\0', sizeof (DWORD));
        memset(&dwMFL, '\0', sizeof (DWORD));
        memset(&dwSysFlags, '\0', sizeof (DWORD));
        //            memset(&szHD, '\0', sizeof (LPCTSTR));

        szHD = szDriveLetters;

        int iType = GetDriveType(szDriveLetters);

        switch (iType) {//TODO add switches for printing in console
            case DRIVE_UNKNOWN:
                break;
            case DRIVE_REMOVABLE:
                bSuccess = GetVolumeInformation(szHD, szVolNameBuff,
                        255, &dwSerial, &dwMFL, &dwSysFlags,
                        szFileSys, 255);

                bSuccess = bSuccess && GetDiskFreeSpaceEx(szHD,
                        (PULARGE_INTEGER) & i64FreeBytesToCaller,
                        (PULARGE_INTEGER) & i64TotalBytes,
                        (PULARGE_INTEGER) & i64FreeBytes);

                /*    printf("Available space to caller = %I64u MB\n",
                              i64FreeBytesToCaller / (1024 * 1024));
                      printf("Total space               = %I64u MB\n",
                              i64TotalBytes / (1024 * 1024));
                      printf("Free space on drive       = %I64u MB\n",
                              i64FreeBytes / (1024 * 1024));
                  }*/

            {
                float dSize = i64TotalBytes / (1024.00 * 1024.00 * 1024.00);
                if (bSuccess) {
                    if (dSize < 1) {
                        dSize = i64TotalBytes / (1024.00 * 1024.00);
                        strDriveSize.Printf("%.2f MB", dSize);
                    } else
                        strDriveSize.Printf("%.2f GB", dSize);
                } else
                    break;
            }
#ifdef NTFS_ONLY
                if (wcscmp(szFileSys, wsNTFS)) {
                    break;
                }
#endif

                strLabel.Printf("%s (%s) [%s]", szVolNameBuff, szHD, strDriveSize);
                ptrListItem->SetText(strLabel);
                ptrListItem->SetData((void*) (new wxString(szHD))); //ovde stoji bas ime drajva, na primer "D:\\"
                ptrListItem->SetImage(iDriveMediaFlashImage);
                ptrListItem->SetId(iListIndex++);
                m_DriveListCtrlPtr->InsertItem(*ptrListItem);

                break;
            case DRIVE_FIXED:
                bSuccess = GetVolumeInformation(szHD, szVolNameBuff,
                        255, &dwSerial, &dwMFL, &dwSysFlags,
                        szFileSys, 255);

                bSuccess = bSuccess && GetDiskFreeSpaceEx(szHD,
                        (PULARGE_INTEGER) & i64FreeBytesToCaller,
                        (PULARGE_INTEGER) & i64TotalBytes,
                        (PULARGE_INTEGER) & i64FreeBytes);

                if (bSuccess) {
                    if (0 == (i64TotalBytes / (1024 * 1024 * 1024)))
                        strDriveSize.Printf("%I64u MB", i64TotalBytes / (1024 * 1024));
                    else
                        strDriveSize.Printf("%I64u GB", i64TotalBytes / (1024 * 1024 * 1024));
                } else
                    break;

#ifdef NTFS_ONLY
                if (wcscmp(szFileSys, wsNTFS)) {
                    break;
                }
#endif
                //dodavanje u listu diskova
                strLabel.Printf("%s (%s) [%s]", szVolNameBuff, szHD, strDriveSize);
                ptrListItem->SetText(strLabel);
                ptrListItem->SetData((void*) (new wxString(szHD))); //ovde stoji bas ime drajva, na primer "D:\\" ovo treba da se prosiri
                ptrListItem->SetImage(iDriveHardDiskImage);
                ptrListItem->SetId(iListIndex++);
                m_DriveListCtrlPtr->InsertItem(*ptrListItem);
                break;
            case DRIVE_REMOTE:
                bSuccess = GetVolumeInformation(szHD, szVolNameBuff,
                        255, &dwSerial, &dwMFL, &dwSysFlags,
                        szFileSys, 255);

                bSuccess = bSuccess && GetDiskFreeSpaceEx(szHD,
                        (PULARGE_INTEGER) & i64FreeBytesToCaller,
                        (PULARGE_INTEGER) & i64TotalBytes,
                        (PULARGE_INTEGER) & i64FreeBytes);

                if (bSuccess) {
                    if (0 == (i64TotalBytes / (1024 * 1024 * 1024)))
                        strDriveSize.Printf("%I64u MB", i64TotalBytes / (1024 * 1024));
                    else
                        strDriveSize.Printf("%I64u GB", i64TotalBytes / (1024 * 1024 * 1024));
                } else
                    break;

#ifdef NTFS_ONLY
                if (wcscmp(szFileSys, wsNTFS)) {
                    break;
                }
#endif

                strLabel.Printf("%s (%s) [%s]", szVolNameBuff, szHD, strDriveSize);
                ptrListItem->SetText(strLabel);
                ptrListItem->SetData((void*) (new wxString(szHD))); //ovde stoji bas ime drajva, na primer "D:\\"
                //                    ptrListItem->SetItemPtrData();
                ptrListItem->SetImage(iDriveRemovableMediaImage);
                ptrListItem->SetId(iListIndex++);
                m_DriveListCtrlPtr->InsertItem(*ptrListItem);

                break;
            case DRIVE_CDROM:
                bSuccess = GetVolumeInformation(szHD, szVolNameBuff,
                        255, &dwSerial, &dwMFL, &dwSysFlags,
                        szFileSys, 255);

                bSuccess = bSuccess && GetDiskFreeSpaceEx(szHD,
                        (PULARGE_INTEGER) & i64FreeBytesToCaller,
                        (PULARGE_INTEGER) & i64TotalBytes,
                        (PULARGE_INTEGER) & i64FreeBytes);

                if (bSuccess) {
                    if (0 == (i64TotalBytes / (1024 * 1024 * 1024)))
                        strDriveSize.Printf("%I64u MB", i64TotalBytes / (1024 * 1024));
                    else
                        strDriveSize.Printf("%I64u GB", i64TotalBytes / (1024 * 1024 * 1024));
                } else
                    break;

#ifdef NTFS_ONLY
                if (wcscmp(szFileSys, wsNTFS)) {
                    break;
                }
#endif

                strLabel.Printf("%s (%s) [%s]", szVolNameBuff, szHD, strDriveSize);
                ptrListItem->SetText(strLabel);
                ptrListItem->SetData((void*) (new wxString(szHD))); //ovde stoji bas ime drajva, na primer "D:\\"
                ptrListItem->SetImage(iDriveOpticalImage);
                ptrListItem->SetId(iListIndex++);
                m_DriveListCtrlPtr->InsertItem(*ptrListItem);

                break;
            case DRIVE_RAMDISK:
                bSuccess = GetVolumeInformation(szHD, szVolNameBuff,
                        255, &dwSerial, &dwMFL, &dwSysFlags,
                        szFileSys, 255);

                bSuccess = bSuccess && GetDiskFreeSpaceEx(szHD,
                        (PULARGE_INTEGER) & i64FreeBytesToCaller,
                        (PULARGE_INTEGER) & i64TotalBytes,
                        (PULARGE_INTEGER) & i64FreeBytes);

                if (bSuccess) {
                    if (0 == (i64TotalBytes / (1024 * 1024 * 1024)))
                        strDriveSize.Printf("%I64u MB", i64TotalBytes / (1024 * 1024));
                    else
                        strDriveSize.Printf("%I64u GB", i64TotalBytes / (1024 * 1024 * 1024));
                } else
                    break;

                if (wcscmp(szFileSys, wsNTFS)) { //da li ovo treba da stoji ovde?
                    break;
                }

                strLabel.Printf("%s (%s) [%s]", szVolNameBuff, szHD, strDriveSize);
                ptrListItem->SetText(strLabel);
                ptrListItem->SetData((void*) (new wxString(szHD))); //ovde stoji bas ime drajva, na primer "D:\\"
                ptrListItem->SetImage(iDriveMediaFlashImage);
                ptrListItem->SetId(iListIndex++);
                m_DriveListCtrlPtr->InsertItem(*ptrListItem);

                break;
            default:
                bOK = false;
        }
        szDriveLetters = &szDriveLetters[wcslen(szDriveLetters) + 1];
    }

    m_DriveListCtrlPtr->Show(true);

    mainSizer->Add(
            m_DriveListCtrlPtr,
            1,
            wxEXPAND,
            5
            );

    SetSizerAndFit(mainSizer);
}

void wxSelectSourceDrivePage::OnWizardPageChanging(wxWizardEvent & event) {
    
    ILanguage *languagePack = ((MyWizard *) GetParent())->GetLanguagePack();
     
    if (event.GetDirection()) { //ako ide na sledecu stranu
        //        static const int max = 1000;

        long item = -1;
        item = m_DriveListCtrlPtr->GetNextItem(item,
                wxLIST_NEXT_ALL,
                wxLIST_STATE_SELECTED);
        if (item == -1) {
            wxMessageDialog *dial = new wxMessageDialog(NULL,
                    languagePack->getWXText(SELECT_SOURCE_DRIVE_PAGE, PLEASE_SELECT_ONE_DRIVE),
                    languagePack->getWXText(GENERAL, ERROR_LABEL), wxOK | wxICON_ERROR);
            dial->ShowModal();
            event.Veto();
            return;
        }

        if (InvalidDiskAccessRights((wxString*) m_DriveListCtrlPtr->GetItemData(item))) {
            wxMessageDialog *dial = new wxMessageDialog(NULL,
                    languagePack->getWXText(SELECT_SOURCE_DRIVE_PAGE, ERROR_LOADING_FILES), languagePack->getWXText(GENERAL, ERROR_LABEL), wxOK | wxICON_ERROR);
            dial->ShowModal();
            event.Veto();
        }
        
        //posle ovog dodaj da pamti i tip particije
        (((MyWizard *) GetParent())->GetMwPtr_strSourceDrive())->Printf("%s", *((wxString*) m_DriveListCtrlPtr->GetItemData(item))); //budz - al jbg

    }
}

void wxSelectSourceDrivePage::OnPageShown(wxWizardEvent& event) {
    wxWindow* NextButton = FindWindowById(wxID_FORWARD, GetParent());
    ILanguage *languagePackLocal = ((MyWizard *) GetParent())->GetLanguagePack();
    
    NextButton->SetLabel(languagePackLocal->getWXText(GENERAL, NEXT_BUTTON_LABEL));
}

void wxSelectSourceDrivePage::OnListItemActivated(wxListEvent & event) {
    //    wxListEvent myEvent(10234, this->GetParent()->GetId());
    //
    //    broj 10234 smo dobili pomocu:
    //            wxEventType peratype = event.GetEventType();
    //        
    //
    //            
    //    // Do send it
    //    if (!this->GetParent()->GetEventHandler()->ProcessEvent(myEvent))
    //        wxLogFatalError("Unable to send event: EVT_WIZARD_PAGE_CHANGING");
}

BEGIN_EVENT_TABLE(wxSelectSourceDrivePage, wxWizardPageSimple)
   EVT_WIZARD_PAGE_CHANGING(wxID_ANY, wxSelectSourceDrivePage::OnWizardPageChanging)
   EVT_LIST_ITEM_ACTIVATED(LIST_CTRL_SOURCE_DRIVES, wxSelectSourceDrivePage::OnListItemActivated) // ne koristi nicemu - hteo sam da uvedem double click za dalje
   EVT_WIZARD_PAGE_SHOWN(wxID_ANY, wxSelectSourceDrivePage::OnPageShown)
        //EVT_WIZARD_CANCEL(wxID_ANY, wxSelectSourceDrivePage::OnWizardCancel)
END_EVENT_TABLE()
