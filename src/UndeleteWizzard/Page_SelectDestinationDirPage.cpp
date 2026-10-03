#include <windows.h>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/wizard.h>
#include <wx/event.h>
#include <wx/button.h>
#include <wx/thread.h>
#include <wx/progdlg.h>
#include <wx/filename.h>

#include "NTFSDrive.h"
#include "MyThreadClass.h"
#include "MyWizzard.h"
#include "Page_SelectDestinationDirPage.h"
#include "languages/LanguageEnums.h"

#include <shlobj.h>
#include <iostream>
#include <string>
#include <sstream>
#include <bits/basic_string.h>

//LALAL
#include "FAT/phmain.h"

//Various pictures
#include "SidePicture2.xpm"

DEFINE_EVENT_TYPE(wxEVT_SELECT_DESTINATION_DRIVE_PAGE_PULSE_DIALOG)
DEFINE_EVENT_TYPE(wxEVT_SELECT_DESTINATION_DRIVE_PAGE_STOP_DIALOG)

wxSelectDestinationDirPage::wxSelectDestinationDirPage(wxWizard *parent) : wxWizardPageSimple(parent) {

    m_bitmap = wxBitmap(SidePicture2_xpm);

    this->SetOwnBackgroundColour(wxColour(210, 222, 255));
    this->SetOwnForegroundColour(wxColour(210, 222, 255));
    this->SetForegroundColour(wxColour(210, 222, 255));
    this->SetBackgroundColour(wxColour(210, 222, 255));

    ILanguage *languagePack = ((MyWizard *) GetParent())->GetLanguagePack();

    wxStaticText *wxSelectDestinationDirMessage = new wxStaticText(this, -1,
            languagePack->getWXText(SELECT_DESTINATION_PAGE, PLEASE_SELECT_DESTINATION));
    wxSelectDestinationDirMessage->SetFont(wxFont(11, wxSWISS, wxNORMAL, wxNORMAL));


    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(wxSelectDestinationDirMessage, 0, wxALL, 5);

    //Fajlovi se snimaju na desktop
    wchar_t localAppData[256];
    SHGetFolderPath(NULL,
            CSIDL_DESKTOPDIRECTORY,
            NULL,
            SHGFP_TYPE_CURRENT,
            localAppData);

    wcscat(localAppData, L"\\RetrievedFiles\\");
    //m_DestDirTextBox = new wxTextCtrl(this, -1, wxT("D:\\RetrievedFiles\\"), wxPoint(-1, -1), wxSize(-1, -1));
    m_DestDirTextBox = new wxTextCtrl(this, -1, localAppData, wxPoint(-1, -1), wxSize(-1, -1), wxTE_LEFT);
    m_buttonBrowseDestinationDir = new wxButton(this, BROWSE_BUTTON_ID, languagePack->getWXText(GENERAL, BROWSE_BUTTON_LABEL));
    m_buttonBrowseDestinationDir->SetBackgroundColour(wxCOLOUR_BUTTON);

    mainSizer->Add(m_DestDirTextBox, 0, wxLEFT | wxEXPAND, 5);
    mainSizer->Add(m_buttonBrowseDestinationDir, 0, wxLEFT, 5);

    SetSizerAndFit(mainSizer);


    Connect(this->GetId(), wxEVT_SELECT_DESTINATION_DRIVE_PAGE_PULSE_DIALOG,
            wxCommandEventHandler(wxSelectDestinationDirPage::OnPulseDialogEvent));

    Connect(this->GetId(), wxEVT_SELECT_DESTINATION_DRIVE_PAGE_STOP_DIALOG,
            wxCommandEventHandler(wxSelectDestinationDirPage::OnStopDialogEvent));
}

void getFilesystemTypeFromDisk(wchar_t * szFileSys, wxString *ptrSourceDrive) {//TODO mozda da vrati neki integer, da znam da li je sve proslo OK
    wchar_t szVolNameBuff[255];
    DWORD dwSerial, dwMFL, dwSysFlags;
    TCHAR lPath[255];
    memset(lPath, '\0', 255 * sizeof (TCHAR));

    const wxChar* myStringChars = ptrSourceDrive->c_str();
    for (int i = 0; i < ptrSourceDrive->Len(); i++) {
        lPath[i] = myStringChars [i];
    }
    lPath[ptrSourceDrive->Len()] = _T('\0');

    GetVolumeInformation(lPath, szVolNameBuff,
            255, &dwSerial, &dwMFL, &dwSysFlags,
            szFileSys, 255);
}

/* Ovde sad ide:
 * - skeniranje direktorijuma,
 * - popunjavanje hash tabele
 * - popunjavanje liste */

void wxSelectDestinationDirPage::OnWizardPageChanging(wxWizardEvent& event) {
    if (event.GetDirection()) { // samo kad se ide na sledecu stranu
        static const int max = 25;

        ILanguage *languagePack = ((MyWizard *) GetParent())->GetLanguagePack();

        // postavi destination dir vrednost u MyWizard klasi
        wxString *ptrDestinationDir = ((MyWizard *) GetParent())->GetMw_ptrDestinationDir();
        ptrDestinationDir->Printf("%s\\", m_DestDirTextBox->GetValue());

        // da li drajv postoji i da li mozes da pises po njemu
        wchar_t tmpDestDir[3 + 1];
        memset(tmpDestDir, '\0', (3 + 1) * sizeof (wchar_t));

        int iRet = GetDriveType((ptrDestinationDir->Left(3).wc_str())); // prva tri karaktera sa pocetka destination dir-a - trebalo bi da bude nesto kao 'D:\\'
        if (!iRet) {
            DWORD dwErrorCode = GetLastError();
            wxLogError("GetDriveType returned error code %d . Check access rights.", dwErrorCode); //sranje - ovde treba provera mesta na hardu i neko lepo preskakanje
        }

        ptrDestinationDir->Replace(wxT("\\\\"), wxT("\\"));

        wxFileName DestDir;

        if (!DestDir.DirExists(*ptrDestinationDir))
            if (!DestDir.Mkdir(*ptrDestinationDir, 0777, wxPATH_MKDIR_FULL)) {
                wxString myErrorMessage;
                myErrorMessage.Printf("Unable to create destination dir\nPlease run with Admin righs"); //TODO
                wxMessageDialog *dial = new wxMessageDialog(NULL,
                        myErrorMessage, languagePack->getWXText(GENERAL, ERROR), wxOK | wxICON_ERROR);
                dial->ShowModal();
                event.Veto();
                return;
            }

        // TODO Ovde napravi proveru da li ti je dest drajv isti kao i source drajv
        wxString *ptrSourceDrive = ((MyWizard *) GetParent())->GetMwPtr_strSourceDrive();
        if (!strncasecmp(ptrDestinationDir->mbc_str(), ptrSourceDrive->mb_str(), 1)) {
            wxMessageDialog *dial = new wxMessageDialog(NULL,
                    languagePack->getWXText(SELECT_DESTINATION_PAGE, SAME_DRIVE_ERROR),
                    languagePack->getWXText(GENERAL, ERROR), wxOK | wxICON_ERROR);
            dial->ShowModal();
            event.Veto();
            return;
        }



        //Initiate log file, from this point on, it is possible to write logs :)
        InitLogFile(*ptrDestinationDir);
        wxLogDebug("Log file initiated. %s:%d[%s]", __FILE__, __LINE__, __FUNCTION__);
        wxLogMessage("Log file initiated.");
        wxLogMessage("Chosen source drive: %s", ((MyWizard *) GetParent())->GetMwPtr_strSourceDrive()->mb_str());
        wxLogMessage("Chosen destination directory: %s", ptrDestinationDir->mb_str());

        
        wchar_t szFileSys[255]; 
        getFilesystemTypeFromDisk(szFileSys, ptrSourceDrive); //budz straaasan, naso na internetu! //TODO podatak o file sistemu treba da se ubaci u Listctrl diskova

        wchar_t wsNTFS[4 + 1] = L"NTFS";
        if (!wcscmp(szFileSys, wsNTFS)) {
            int i = 0;
            i++;
        } else if (!wcsncmp(szFileSys, L"FAT",3)) {
            wxString *wxDrive = ((MyWizard *) GetParent())->GetMwPtr_strSourceDrive();
            photorecz(wxDrive->ToStdString().at(0), ptrDestinationDir->ToStdString().c_str());
            return;
        }

        // tek sad pravis dijalog                
        wxProgressDialog dialog("Scanning the selected source drive.",
                // "Reserve" enough space for the multiline
                // messages below, we'll change it anyhow
                // immediately in the loop below
                wxString("Scanning your hard drive for deleted files") + "\n\n\n\n",
                max, // range
                this, // parent
                // wxPD_APP_MODAL |
                wxPD_CAN_ABORT |
                //                wxPD_CAN_SKIP | // testing purposes - should be disabled in prod version
                wxPD_AUTO_HIDE | // -- try this as well
                wxPD_ELAPSED_TIME |
                wxPD_SMOOTH // - makes indeterminate mode bar on WinXP very small
                );

        wxLogMessage("Progress Dialog created.");

        //        dialog.SetOwnBackgroundColour(wxColour(249, 213, 175));
        //        dialog.SetOwnForegroundColour(wxColour(249, 113, 175));
        //        dialog.SetForegroundColour(wxColour(149, 213, 175));
        //        dialog.SetBackgroundColour(wxColour(249, 213, 175));

        // napravi thread, dodeli mu sve sto mu treba i pokreni ga
        wxLogMessage("Setting up thread variables.");
        MyThreadLoadFilesFromDiskNTFS *thLoadFiles = new MyThreadLoadFilesFromDiskNTFS(this);
        thLoadFiles->SetNTFSDrivePtr(((MyWizard *) GetParent())->GetPtrCNTFSDrive());
        thLoadFiles->SetFinfoHashPtr(((MyWizard *) GetParent())->GetPtrFileInfoHash());
        thLoadFiles->SetListControlPtr(((MyWizard *) GetParent())->GetPtrSelectedFiles());
        thLoadFiles->SetDestinationDirPtr(ptrDestinationDir);
        thLoadFiles->SetCancelProgresControl(false);
        thLoadFiles->SetSourceDrive(((MyWizard *) GetParent())->GetMwPtr_strSourceDrive());
        thLoadFiles->SetMutexLockPtr(((MyWizard *) GetParent())->GetPtrListMutex()); // ovo nicemu ne sluzi        

        if (thLoadFiles->Create() != wxTHREAD_NO_ERROR) {
            wxLogFatalError(languagePack->getWXText(SELECT_DESTINATION_PAGE, CANT_CREATE_THREADS_ERROR));
        }
        wxLogMessage("LoadFiles Thread created.");

        thLoadFiles->Run(); // krenuo thread, sad ce da ode u Entry() fju. !!! ovo vraca fajlove! zoran
        wxLogMessage("LoadFiles Thread started");

        bool cont = true; // ovaj se koristi ako je stisnut cancel - to moram da vidim gde sta treba da implementiram... 
        // moguce da cu na ovoj strani da stavim cancel a na sledecoj oba, i skip i cancel hmmm...         
        this->mw_bRunDialog = true;
        this->mw_sDialogString = "Looking for deleted files...";

        // progres dialog se vriti a u drugom threadu se citaju disk i pune liste
        // kad se promeni stanje, opale se eventi koji menjaju this->mw_bRunDialog i this->mw_sDialogString
        wxLogMessage("Entering progressDialog dialog loop.");
        while (this->mw_bRunDialog) {
            // will be set to true if "Skip" button was clicked           
            bool skip = false;
            cont = dialog.Pulse(this->mw_sDialogString, &skip);

            // each skip will move progress about quarter forward
            if (skip) // ovo se ne koristi
                cont = false;

            if (!cont) {
                if (wxMessageBox(languagePack->getWXText(SELECT_DESTINATION_PAGE, DO_YOU_REALLY_WANT_TO_CANCEL),
                        languagePack->getWXText(GENERAL, PROGRESS_DIALOG_QUESTION), // caption
                        wxYES_NO | wxICON_QUESTION) == wxYES) {
                    thLoadFiles->SetCancelProgresControl(true);
                    event.Veto();
                    break;
                } else {
                    cont = true;
                    dialog.Resume();
                }
            }
            wxMilliSleep(200);
        }

        // objavimo da smo prekinuli citanje
        if (!cont) {
            wxLogStatus(wxT("Progress dialog aborted!"));
        }
    }
}

bool wxSelectDestinationDirPage::InitLogFile(wxString wxDestinationDir) {
    wxString wxsCompleteLogFileName;
    wxString wxsLogFileName;

    SYSTEMTIME time;
    GetLocalTime(&time);

    wxsLogFileName.Printf("UndeleteLogFile_%d.%d.%d@%dh%dm%ds.log", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
    wxsCompleteLogFileName = wxDestinationDir + wxsLogFileName;

    //TODO: napravi log fajl
    FILE *logfile = fopen(wxsCompleteLogFileName.mb_str(), "w");
    if (!logfile)
        return false;

    wxLog *logger = new wxLogStderr(logfile);
    wxLog::SetActiveTarget(logger);

    return true;
}

void wxSelectDestinationDirPage::OnPulseDialogEvent(wxCommandEvent & event) {
    this->mw_sDialogString = event.GetString();
}

void wxSelectDestinationDirPage::OnStopDialogEvent(wxCommandEvent & event) {
    this->mw_bRunDialog = false;
}

void wxSelectDestinationDirPage::ChooseDestination(wxCommandEvent & event) {

    wxDirDialog * openDirDialog = new wxDirDialog(this);

    if (openDirDialog->ShowModal() == wxID_OK) {
        wxString *fileName = ((MyWizard *) GetParent())->GetMw_ptrDestinationDir();
        fileName->Printf("%s\\RetrievedFiles\\", openDirDialog->GetPath());
        fileName->Replace(wxT("\\\\"), wxT("\\"));
        m_DestDirTextBox->Clear();
        m_DestDirTextBox->SetValue(*fileName);
    }
}

void wxSelectDestinationDirPage::OnShowNotification(wxCommandEvent & event) {
    wxMessageDialog *dial = new wxMessageDialog(NULL, event.GetString(), wxT("Info"), wxOK | wxICON_INFORMATION);
    dial->ShowModal();
}

void wxSelectDestinationDirPage::OnDialogColourChanging(wxSysColourChangedEvent & event) {
    printf("sranje");
}

void wxSelectDestinationDirPage::OnPageShown(wxWizardEvent& event) {
    wxWindow* NextButton = FindWindowById(wxID_FORWARD, GetParent());
    ILanguage *languagePackLocal = ((MyWizard *) GetParent())->GetLanguagePack();

    NextButton->SetLabel(languagePackLocal->getWXText(GENERAL, NEXT_BUTTON_LABEL));
}


BEGIN_EVENT_TABLE(wxSelectDestinationDirPage, wxWizardPageSimple)
EVT_BUTTON(BROWSE_BUTTON_ID, wxSelectDestinationDirPage::ChooseDestination)
EVT_WIZARD_PAGE_CHANGING(wxID_ANY, wxSelectDestinationDirPage::OnWizardPageChanging)
EVT_SYS_COLOUR_CHANGED(wxSelectDestinationDirPage::OnDialogColourChanging)
EVT_WIZARD_PAGE_SHOWN(wxID_ANY, wxSelectDestinationDirPage::OnPageShown)
END_EVENT_TABLE()
