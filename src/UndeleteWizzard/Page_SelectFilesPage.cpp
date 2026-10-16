#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/progdlg.h>
#include <wx/wizard.h>
#include <wx/dir.h>
#include <wx/tglbtn.h>
//#include <wx/srchctrl.h>

#include "NTFSDrive.h"
#include "MyThreadClass.h"
#include "MyWizzard.h"
#include "Page_SelectFilesPage.h"
#include "languages/LanguageEnums.h"

#include "resources/SidePicture3.xpm"

DEFINE_EVENT_TYPE(wxEVT_SELECT_FILES_PAGE_PULSE_DIALOG)
DEFINE_EVENT_TYPE(wxEVT_SELECT_FILES_PAGE_STOP_DIALOG)
DEFINE_EVENT_TYPE(wxEVT_SELECT_FILES_SEARCH_ACTION)

wxSelectFilesPage::wxSelectFilesPage(wxWizard *parent) : wxWizardPageSimple(parent) {
    m_bitmap = wxBitmap(SidePicture3_xpm);

    this->SetOwnBackgroundColour(wxColour(210, 222, 255));
    this->SetOwnForegroundColour(wxColour(210, 222, 255));
    this->SetForegroundColour(wxColour(210, 222, 255));
    this->SetBackgroundColour(wxColour(210, 222, 255));

    wxBoxSizer *sizerHoldingEverythingBelowTheMessage = new wxBoxSizer(wxHORIZONTAL); //1
    wxBoxSizer *sizerHoldingButtonsAndSearchAndTheList = new wxBoxSizer(wxVERTICAL); //2
    wxBoxSizer *sizerHoldingButtonsAboveTheList = new wxBoxSizer(wxHORIZONTAL); //4
    wxBoxSizer *sizerHoldingTheButtonsOnTheRightSide = new wxBoxSizer(wxVERTICAL); //3

    ILanguage *languagePack = ((MyWizard *) GetParent())->GetLanguagePack();

    wxButton *wxSellectAllButton = new wxButton(this, SELECT_ALL_BUTTON, languagePack->getWXText(SELECT_FILES_PAGE, SELECT_ALL_BUTTON_LABEL));
    wxButton *wxUnselectAllButton = new wxButton(this, UNSELECT_ALL_BUTTON, languagePack->getWXText(SELECT_FILES_PAGE, UNSELECT_ALL_BUTTON_LABEL));

    //ovu promenjivu sam definisao kao clan klase da bi mogao lakse da je dohvatim u search funkciji - ako se bude slala kao evenet nekom threadu - treba je definisati kao lokalnu
    m_wxSearchCtrlSearchList = new wxSearchCtrl(this, SEARCH_CONTROL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_wxSearchCtrlSearchList->ShowCancelButton(true);

    wxFilterImages = new wxToggleButton(this, FILTER_IMAGES_BUTTON, languagePack->getWXText(SELECT_FILES_PAGE, ALL_IMAGES_BUTTON_LABEL), wxDefaultPosition, wxSize(80, 25));
    wxFilterVideo = new wxToggleButton(this, FILTER_VIDEO_BUTTON, languagePack->getWXText(SELECT_FILES_PAGE, ALL_VIDEOS_BUTTON_LABEL), wxDefaultPosition, wxSize(80, 25));
    wxFilterMusic = new wxToggleButton(this, FILTER_MUSIC_BUTTON, languagePack->getWXText(SELECT_FILES_PAGE, ALL_MUSIC_BUTTON_LABEL), wxDefaultPosition, wxSize(80, 25));


    // set nice colors :)
    wxSellectAllButton->SetOwnBackgroundColour(wxColour(63, 119, 246)); //darker blue
    wxSellectAllButton->SetOwnForegroundColour(wxColour(255, 255, 255)); // white

    wxUnselectAllButton->SetOwnBackgroundColour(wxColour(63, 119, 246)); //darker blue
    wxUnselectAllButton->SetOwnForegroundColour(wxColour(255, 255, 255)); // white

    sizerHoldingTheButtonsOnTheRightSide->Add(wxFilterImages, 0, wxTOP, 35);
    sizerHoldingTheButtonsOnTheRightSide->Add(wxFilterVideo, 0, wxTOP, 4);
    sizerHoldingTheButtonsOnTheRightSide->Add(wxFilterMusic, 0, wxTOP, 4);

    sizerHoldingButtonsAboveTheList->Add(wxSellectAllButton, 0, wxLEFT, 0);
    sizerHoldingButtonsAboveTheList->Add(wxUnselectAllButton, 0, wxLEFT, 5);
    sizerHoldingButtonsAboveTheList->Add(m_wxSearchCtrlSearchList, 1, wxEXPAND | wxLEFT, 80);


    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    wxSelectFilesMessage = new wxStaticText(this, -1,
            languagePack->getWXText(SELECT_FILES_PAGE, SELECT_FILES));

    wxSelectFilesMessage->SetFont(wxFont(11, wxSWISS, wxNORMAL, wxNORMAL));

    // virtual list - filled by wxSelectDestinationDirPage when the scan is done
    m_lcFilesInSelectedDrive = new CDeletedFilesList(this, LIST_CTRL_DELETED_FILES, wxPoint(-1, -1), wxSize(600, 400));
    ((MyWizard *) GetParent())->SetPtrSelectedFiles(m_lcFilesInSelectedDrive);


    sizerHoldingButtonsAndSearchAndTheList->Add(sizerHoldingButtonsAboveTheList, 0, wxEXPAND, 0);
    sizerHoldingButtonsAndSearchAndTheList->Add(m_lcFilesInSelectedDrive, 1, wxEXPAND | wxTOP, 5);
    //    lowerSizer->SetItemMinSize(m_lcFilesInSelectedDrive, 600, 600);
    //    sizerHoldingButtonsAndSearchAndTheList->SetMinSize(700, 700);
    //    sizerHoldingButtonsAndSearchAndTheList->SetItemMinSize(m_lcFilesInSelectedDrive, 700, 700);
    //    lowerSizer->SetItemMinSize(1, 700, 700);

    sizerHoldingEverythingBelowTheMessage->Add(sizerHoldingButtonsAndSearchAndTheList, 1, wxEXPAND | wxTOP, 5);
    sizerHoldingEverythingBelowTheMessage->Add(sizerHoldingTheButtonsOnTheRightSide, 0, wxEXPAND | wxLEFT, 5);

    mainSizer->Add(wxSelectFilesMessage, 0, wxALL, 4);
    mainSizer->Add(sizerHoldingEverythingBelowTheMessage, 1, wxEXPAND, 0);

    mainSizer->SetMinSize(800, 800);

    Connect(this->GetId(), wxEVT_SELECT_FILES_PAGE_PULSE_DIALOG,
            wxCommandEventHandler(wxSelectFilesPage::OnPulseDialogEvent));

    Connect(this->GetId(), wxEVT_SELECT_FILES_PAGE_STOP_DIALOG,
            wxCommandEventHandler(wxSelectFilesPage::OnStopDialogEvent));

    Connect(SELECT_ALL_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(wxSelectFilesPage::OnSelectAllClick));

    Connect(UNSELECT_ALL_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(wxSelectFilesPage::OnUnselectAllClick));

    Connect(FILTER_IMAGES_BUTTON, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
            wxCommandEventHandler(wxSelectFilesPage::OnFilterImagesClick));

    Connect(FILTER_VIDEO_BUTTON, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
            wxCommandEventHandler(wxSelectFilesPage::OnFilterVideosClick));

    Connect(FILTER_MUSIC_BUTTON, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
            wxCommandEventHandler(wxSelectFilesPage::OnFilterMusicClick));

    Connect(LIST_CTRL_DELETED_FILES, wxEVT_COMMAND_LIST_ITEM_FOCUSED, //radi do nekle, ne bas savrseno
            wxCommandEventHandler(wxSelectFilesPage::OnFilesListClick));

    Connect(LIST_CTRL_DELETED_FILES, wxEVT_COMMAND_LIST_COL_CLICK, //radi do nekle, ne bas savrseno
            wxCommandEventHandler(wxSelectFilesPage::OnListColumnClick));

    SetSizerAndFit(mainSizer);
}

void wxSelectFilesPage::OnWizardPageChanging(wxWizardEvent& event) {
    if (event.GetDirection()) {

#ifdef DEMO_BUILD
        wxMessageDialog *dial = new wxMessageDialog(NULL,
                wxT("This is a DEMO version!\n File saving is not possible.\n To buy full version, visit www.getmyfilesback.com\n"), wxT("Error"), wxOK | wxICON_ERROR);
        dial->ShowModal(); 
        event.Veto();
        return;
#else        
        ILanguage *languagePack = ((MyWizard *) GetParent())->GetLanguagePack();
        int *ptrNumberOfSelectedFiles = ((MyWizard *) GetParent())->GetMw_ptriNumberOfSelectedFiles();

        // collect the selection here, in the GUI thread - the copy thread gets the list of files, not the control
        std::vector<t_FileInfo *> vFilesToCopy;
        m_lcFilesInSelectedDrive->GetSelectedFiles(vFilesToCopy);
        *ptrNumberOfSelectedFiles = (int) vFilesToCopy.size();
        //        (*this->m_NumberOfSelectedFiles) = ;//sto je ovo glupo resenje - katastrofa - ne treba mu jos jedna referenca na isti pointer

        if (vFilesToCopy.empty()) {
            wxMessageDialog dial(NULL, wxT("Please select the files you want to retrieve."), wxT("Info"), wxOK | wxICON_INFORMATION);
            dial.ShowModal();
            event.Veto();
            return;
        }

        wxProgressDialog dialog("Scanning the selected source drive.",
                // "Reserve" enough space for the multiline
                // messages below, we'll change it anyhow
                // immediately in the loop below
                wxString("Scanning your hard drive for deleted files...") + "\n\n\n\n",
                *ptrNumberOfSelectedFiles, // range
                this, // parent
                // wxPD_APP_MODAL |
                wxPD_CAN_ABORT |
                //                wxPD_CAN_SKIP | // testing purposes - should be disabled in prod version
                wxPD_AUTO_HIDE | // -- try this as well
                wxPD_ELAPSED_TIME |
                wxPD_SMOOTH // - makes indeterminate mode bar on WinXP very small
                );


        MyThreadCopyFiles *thCopyFiles = new MyThreadCopyFiles(this);

        thCopyFiles->SetNTFSDrivePtr(((MyWizard *) GetParent())->GetPtrCNTFSDrive());
        thCopyFiles->SetFinfoHashPtr(((MyWizard *) GetParent())->GetPtrFileInfoHash());
        thCopyFiles->SetFilesToCopy(vFilesToCopy);
        thCopyFiles->SetDestinationDirPtr(((MyWizard *) GetParent())->GetMw_ptrDestinationDir());
        thCopyFiles->SetMutexLockPtr(((MyWizard *) GetParent())->GetPtrListMutex()); // ovo nicemu ne sluzi
        thCopyFiles->SetCancelProgresControl(false);
        thCopyFiles->SetMwPtr_strSelectedDir(((MyWizard *) GetParent())->GetMw_ptrDestinationDir());
        thCopyFiles->SetIptrNumberOfCopiedFiles(((MyWizard *) GetParent())->GetIptrNumberOfCopiedFiles());

        if (thCopyFiles->Create() != wxTHREAD_NO_ERROR) {
            delete thCopyFiles;
            wxMessageDialog dial(NULL, languagePack->getWXText(SELECT_DESTINATION_PAGE, CANT_CREATE_THREADS_ERROR),
                    languagePack->getWXText(GENERAL, ERROR), wxOK | wxICON_ERROR);
            dial.ShowModal();
            event.Veto();
            return;
        }

        SetDialogString("Copying deleted files...");
        this->m_NumberOfFileBeingCopied = 0;
        this->mw_bRunDialog = true;

        thCopyFiles->Run();

        bool cont = true; // ovaj se koristi ako je stisnut cancel - to moram da vidim gde sta treba da implementiram...
        // moguce da cu na ovoj strani da stavim cancel a na sledecoj oba, i skip i cancel hmmm...
        while (this->mw_bRunDialog) {
            // will be set to true if "Skip" button was clicked
            bool skip = false;
            cont = dialog.Update(this->m_NumberOfFileBeingCopied, GetDialogString(), &skip);

            // each skip will move progress about quarter forward
            if (skip) // ovo se ne koristi
                cont = false;

            if (!cont) {
                if (wxMessageBox(languagePack->getWXText(SELECT_DESTINATION_PAGE, DO_YOU_REALLY_WANT_TO_CANCEL),
                        languagePack->getWXText(GENERAL, PROGRESS_DIALOG_QUESTION), // caption
                        wxYES_NO | wxICON_QUESTION) == wxYES) {
                    thCopyFiles->SetCancelProgresControl(true); //stop processing in the thread
                    event.Veto();
                    break;
                } else {
                    cont = true;
                    dialog.Resume();
                }
            }
            wxMilliSleep(100);
        }

        // a cancelled copy stops within 1 MB of the current file - wait for it, so nothing else reads the drive meanwhile
        {
            wxBusyCursor busy;
            thCopyFiles->Wait();
        }
        delete thCopyFiles;
        wxLog::FlushActive(); // the thread's log lines

        if (!cont) {
            wxLogStatus(languagePack->getWXText(GENERAL, PROGRESS_DIALOG_ABORTED));
        } else {
            wxLogStatus(wxT("Countdown from %d finished"), *ptrNumberOfSelectedFiles);
        }
#endif
    }
}

void wxSelectFilesPage::OnStopDialogEvent(wxCommandEvent & event) {
    this->mw_bRunDialog = false;
}

// runs in the copy thread (the event is sent with ProcessEvent) - the text is shared with the GUI thread

void wxSelectFilesPage::OnPulseDialogEvent(wxCommandEvent & event) {
    int *ptrNumberOfSelectedFiles = ((MyWizard *) GetParent())->GetMw_ptriNumberOfSelectedFiles();
    SetDialogString(wxString::Format("Copying %d/%d\nFilename: %s", event.GetInt(), *ptrNumberOfSelectedFiles, event.GetString()));
    this->m_NumberOfFileBeingCopied = event.GetInt();
}

void wxSelectFilesPage::SetDialogString(const wxString &sText) {
    wxCriticalSectionLocker lock(m_csDialogString);
    this->mw_sDialogString = wxString(sText.wc_str()); // deep copy - no buffer shared between the threads
}

wxString wxSelectFilesPage::GetDialogString() {
    wxCriticalSectionLocker lock(m_csDialogString);
    return wxString(this->mw_sDialogString.wc_str());
}

void wxSelectFilesPage::OnSelectAllClick(wxCommandEvent & event) {
    m_lcFilesInSelectedDrive->SelectAll(true);

    wxFilterImages->SetValue(true);
    wxFilterVideo->SetValue(true);
    wxFilterMusic->SetValue(true);
}

void wxSelectFilesPage::OnUnselectAllClick(wxCommandEvent & event) {
    m_lcFilesInSelectedDrive->SelectAll(false);

    wxFilterImages->SetValue(false);
    wxFilterVideo->SetValue(false);
    wxFilterMusic->SetValue(false);
}

static bool _hasExtension(const wchar_t *szFilename, const char *const *extensions, int count) {
    wxString name(szFilename);
    name.MakeLower(); // -> mala slova
    for (int i = 0; i < count; i++)
        if (name.EndsWith(wxString(".") + extensions[i]))
            return true;
    return false;
}

static bool _isImage(const wchar_t *szFilename) {
    static const char *const ImageExtensions[] = {"jpg", "jpeg", "gif", "png", "bmp", "tif", "tiff", "heic"};
    return _hasExtension(szFilename, ImageExtensions, sizeof (ImageExtensions) / sizeof (ImageExtensions[0]));
}

static bool _isVideo(const wchar_t *szFilename) {
    static const char *const VideoExtensions[] = {"avi", "mov", "wmv", "mp4", "mkv", "m4v", "mpg", "mpeg"};
    return _hasExtension(szFilename, VideoExtensions, sizeof (VideoExtensions) / sizeof (VideoExtensions[0]));
}

static bool _isMusic(const wchar_t *szFilename) {
    static const char *const MusicExtensions[] = {"mp3", "wma", "wav", "flac", "m4a", "ogg"};
    return _hasExtension(szFilename, MusicExtensions, sizeof (MusicExtensions) / sizeof (MusicExtensions[0]));
}

// select (or unselect) every listed file of one type, e.g. all images

void wxSelectFilesPage::SelectFilesOfType(bool (*isOfType)(const wchar_t *), bool bSelect) {
    long lCount = m_lcFilesInSelectedDrive->GetFileCount();
    for (long lItem = 0; lItem < lCount; lItem++)
        if (isOfType(m_lcFilesInSelectedDrive->GetFile(lItem)->sFilename.c_str()))
            m_lcFilesInSelectedDrive->SelectItem(lItem, bSelect);
}

void wxSelectFilesPage::OnFilterImagesClick(wxCommandEvent & event) {
    SelectFilesOfType(_isImage, wxFilterImages->GetValue());
}

void wxSelectFilesPage::OnFilterVideosClick(wxCommandEvent & event) {
    SelectFilesOfType(_isVideo, wxFilterVideo->GetValue());
}

void wxSelectFilesPage::OnFilterMusicClick(wxCommandEvent & event) {
    SelectFilesOfType(_isMusic, wxFilterMusic->GetValue());
}

void wxSelectFilesPage::OnSearchAction(wxCommandEvent & event) {
    // for an empty string the list shows all files again
    m_lcFilesInSelectedDrive->SetNameFilter(event.GetString());
    UpdateFoundFilesLabel();
}

void wxSelectFilesPage::OnSearchCancel(wxCommandEvent & event) {
    m_wxSearchCtrlSearchList->Clear();
    m_lcFilesInSelectedDrive->SetNameFilter(wxEmptyString);
    UpdateFoundFilesLabel();
}

void wxSelectFilesPage::OnFilesListClick(wxCommandEvent & event) {
    wxFilterImages->SetValue(false);
    wxFilterVideo->SetValue(false);
    wxFilterMusic->SetValue(false);
}

void wxSelectFilesPage::OnListColumnClick(wxCommandEvent & event) {
    // the same column again reverses the order
    m_lcFilesInSelectedDrive->SortByColumn(((wxListEvent *) & event)->GetColumn());
}

void wxSelectFilesPage::UpdateFoundFilesLabel() {
    ILanguage *languagePackLocal = ((MyWizard *) GetParent())->GetLanguagePack();
    int itemNumber = (int) m_lcFilesInSelectedDrive->GetFileCount();
    wxSelectFilesMessage->SetLabel(wxString::Format(languagePackLocal->getWXText(SELECT_FILES_PAGE, SELECT_FILES_OTHER), itemNumber));
}

void wxSelectFilesPage::OnPageShown(wxWizardEvent& event) {

    ILanguage *languagePackLocal = ((MyWizard *) GetParent())->GetLanguagePack();
    wxWindow* NextButton = FindWindowById(wxID_FORWARD, GetParent());
    NextButton->SetLabel(languagePackLocal->getWXText(GENERAL, RETRIEVE_BUTTON_LABEL));

    UpdateFoundFilesLabel();

    //set the next button label




}

BEGIN_EVENT_TABLE(wxSelectFilesPage, wxWizardPageSimple)
EVT_WIZARD_PAGE_CHANGING(wxID_ANY, wxSelectFilesPage::OnWizardPageChanging)
EVT_WIZARD_PAGE_SHOWN(wxID_ANY, wxSelectFilesPage::OnPageShown)
EVT_SEARCHCTRL_SEARCH_BTN(SEARCH_CONTROL, wxSelectFilesPage::OnSearchAction)
EVT_TEXT_ENTER(SEARCH_CONTROL, wxSelectFilesPage::OnSearchAction)
EVT_SEARCHCTRL_CANCEL_BTN(SEARCH_CONTROL, wxSelectFilesPage::OnSearchCancel)
//EVT_WIZARD_CANCEL(wxID_ANY, wxSelectFilesPage::OnWizardCancel)
END_EVENT_TABLE()




// TODO: Nacin selektovanja fajlova. Da ne mora Ctrl da se drzi