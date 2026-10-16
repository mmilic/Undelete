#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/progdlg.h>
#include <wx/wizard.h>
#include <wx/dir.h>
#include <wx/tglbtn.h>
#include <wx/mstream.h>
#include <wx/image.h>
//#include <wx/srchctrl.h>

#include "NTFSDrive.h"
#include "FileContents.h"
#include "MyThreadClass.h"
#include "MyWizzard.h"
#include "Page_SelectFilesPage.h"
#include "languages/LanguageEnums.h"

#include "resources/SidePicture3.xpm"

DEFINE_EVENT_TYPE(wxEVT_SELECT_FILES_PAGE_PULSE_DIALOG)
DEFINE_EVENT_TYPE(wxEVT_SELECT_FILES_PAGE_STOP_DIALOG)

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

    // preview of the focused file, read straight from the drive - if it doesn't decode, the data is damaged
    m_bmpPreview = new wxStaticBitmap(this, -1, wxNullBitmap, wxDefaultPosition, wxSize(PREVIEW_SIZE, PREVIEW_SIZE));
    m_txtPreview = new wxStaticText(this, -1, wxT("Select an image\nto see a preview."), wxDefaultPosition,
            wxSize(PREVIEW_SIZE, -1), wxST_NO_AUTORESIZE);
    m_txtPreview->SetForegroundColour(wxColour(60, 60, 60));
    sizerHoldingTheButtonsOnTheRightSide->Add(new wxStaticText(this, -1, wxT("Preview:")), 0, wxTOP, 20);
    sizerHoldingTheButtonsOnTheRightSide->Add(m_bmpPreview, 0, wxTOP, 4);
    sizerHoldingTheButtonsOnTheRightSide->Add(m_txtPreview, 0, wxTOP, 4);

    m_previewTimer.SetOwner(this, PREVIEW_TIMER);
    m_bCopying = false;

    sizerHoldingButtonsAboveTheList->Add(wxSellectAllButton, 0, wxLEFT, 0);
    sizerHoldingButtonsAboveTheList->Add(wxUnselectAllButton, 0, wxLEFT, 5);
    sizerHoldingButtonsAboveTheList->Add(m_wxSearchCtrlSearchList, 1, wxEXPAND | wxLEFT, 80);

    // filters: which files the list shows (the buttons on the right select files, these hide them)
    m_choiceRecoverability = new wxChoice(this, FILTER_RECOVERABILITY_CHOICE);
    for (int i = 0; i < RECOVERABILITY_FILTER_COUNT; i++)
        m_choiceRecoverability->Append(RecoverabilityFilterLabel(i));
    m_choiceRecoverability->SetSelection(RECOVERABILITY_FILTER_ALL);

    m_choiceType = new wxChoice(this, FILTER_TYPE_CHOICE);
    for (int i = FILE_TYPE_ANY; i < FILE_TYPE_COUNT; i++) // item index == FILE_TYPE_*
        m_choiceType->Append(FileTypeFilterLabel(i));
    m_choiceType->SetSelection(FILE_TYPE_ANY);

    m_txtShowing = new wxStaticText(this, -1, wxEmptyString);
    m_txtShowing->SetForegroundColour(wxColour(60, 60, 60));

    wxButton *wxCheckContentsButton = new wxButton(this, CHECK_CONTENTS_BUTTON, wxT("Check contents"));
    wxCheckContentsButton->SetToolTip(wxT("Reads the start of each selected file (or of every shown file, if none is selected)\n"
            "and checks that it still looks like its type - e.g. that a .jpg still holds a JPEG image."));

    wxBoxSizer *sizerHoldingTheFilters = new wxBoxSizer(wxHORIZONTAL);
    sizerHoldingTheFilters->Add(new wxStaticText(this, -1, wxT("Show:")), 0, wxALIGN_CENTER_VERTICAL, 0);
    sizerHoldingTheFilters->Add(m_choiceRecoverability, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    sizerHoldingTheFilters->Add(new wxStaticText(this, -1, wxT("Type:")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 15);
    sizerHoldingTheFilters->Add(m_choiceType, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    // the filter row is as wide as the list - the button goes to the column on the right, under the type buttons
    sizerHoldingTheButtonsOnTheRightSide->Insert(3, wxCheckContentsButton, 0, wxEXPAND | wxTOP, 12);


    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    wxSelectFilesMessage = new wxStaticText(this, -1,
            languagePack->getWXText(SELECT_FILES_PAGE, SELECT_FILES));

    wxSelectFilesMessage->SetFont(wxFont(11, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    // virtual list - filled by wxSelectDestinationDirPage when the scan is done
    m_lcFilesInSelectedDrive = new CDeletedFilesList(this, LIST_CTRL_DELETED_FILES, wxPoint(-1, -1), wxSize(600, 400));
    ((MyWizard *) GetParent())->SetPtrSelectedFiles(m_lcFilesInSelectedDrive);


    sizerHoldingButtonsAndSearchAndTheList->Add(sizerHoldingButtonsAboveTheList, 0, wxEXPAND, 0);
    sizerHoldingButtonsAndSearchAndTheList->Add(sizerHoldingTheFilters, 0, wxEXPAND | wxTOP, 5);
    sizerHoldingButtonsAndSearchAndTheList->Add(m_lcFilesInSelectedDrive, 1, wxEXPAND | wxTOP, 5);
    sizerHoldingButtonsAndSearchAndTheList->Add(m_txtShowing, 0, wxEXPAND | wxTOP, 3); // "Showing 12 of 340 files"
    //    lowerSizer->SetItemMinSize(m_lcFilesInSelectedDrive, 600, 600);
    //    sizerHoldingButtonsAndSearchAndTheList->SetMinSize(700, 700);
    //    sizerHoldingButtonsAndSearchAndTheList->SetItemMinSize(m_lcFilesInSelectedDrive, 700, 700);
    //    lowerSizer->SetItemMinSize(1, 700, 700);

    sizerHoldingEverythingBelowTheMessage->Add(sizerHoldingButtonsAndSearchAndTheList, 1, wxEXPAND | wxTOP, 5);
    sizerHoldingEverythingBelowTheMessage->Add(sizerHoldingTheButtonsOnTheRightSide, 0, wxEXPAND | wxLEFT, 5);

    mainSizer->Add(wxSelectFilesMessage, 0, wxALL, 4);
    mainSizer->Add(sizerHoldingEverythingBelowTheMessage, 1, wxEXPAND, 0);

    // the list, its 7 columns and the column of buttons/preview - and the whole wizard still fits a 1366x768 screen
    mainSizer->SetMinSize(FromDIP(wxSize(1000, 540)));

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

    Connect(FILTER_RECOVERABILITY_CHOICE, wxEVT_COMMAND_CHOICE_SELECTED,
            wxCommandEventHandler(wxSelectFilesPage::OnFilterChoice));

    Connect(FILTER_TYPE_CHOICE, wxEVT_COMMAND_CHOICE_SELECTED,
            wxCommandEventHandler(wxSelectFilesPage::OnFilterChoice));

    Connect(CHECK_CONTENTS_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(wxSelectFilesPage::OnCheckContentsClick));

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
        MyWizard *pWizard = (MyWizard *) GetParent();
        ILanguage *languagePack = pWizard->GetLanguagePack();
        int *ptrNumberOfSelectedFiles = pWizard->GetMw_ptriNumberOfSelectedFiles();
        int *ptrNumberOfCopiedFiles = pWizard->GetIptrNumberOfCopiedFiles();

        // collect the selection here, in the GUI thread - the copy thread gets the list of files, not the control
        std::vector<t_FileInfo *> vFilesToCopy;
        m_lcFilesInSelectedDrive->GetSelectedFiles(vFilesToCopy);
        *ptrNumberOfSelectedFiles = (int) vFilesToCopy.size();

        if (vFilesToCopy.empty()) {
            wxMessageDialog dial(this, wxT("Please select the files you want to retrieve."), wxT("Info"), wxOK | wxICON_INFORMATION);
            dial.ShowModal();
            event.Veto();
            return;
        }

        // overwritten files: other files now use their clusters - warn before spending time on them
        int iOverwritten = 0;
        LONGLONG llTotalBytes = 0;
        for (size_t i = 0; i < vFilesToCopy.size(); i++) {
            if (vFilesToCopy[i]->iRecoverability == RECOVERABILITY_OVERWRITTEN)
                iOverwritten++;
            llTotalBytes += vFilesToCopy[i]->n64RealSize;
        }
        if (iOverwritten) {
            wxString sQuestion;
            if (iOverwritten == (int) vFilesToCopy.size())
                sQuestion.Printf("The selected file%s marked as overwritten: other files are now stored where %s data was.\n\n"
                    "%s will most likely be unreadable. Recover anyway?",
                    iOverwritten == 1 ? " is" : "s are", iOverwritten == 1 ? "its" : "their", iOverwritten == 1 ? "It" : "They");
            else
                sQuestion.Printf("%d of the %d selected files are marked as overwritten: other files are now stored where their data was. "
                    "They will most likely be unreadable.\n\n"
                    "Recover them anyway?\n(Choose No to go back and change the selection.)",
                    iOverwritten, (int) vFilesToCopy.size());
            if (wxMessageBox(sQuestion, wxT("Overwritten files"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING, this) != wxYES) {
                event.Veto();
                return;
            }
        }
        *pWizard->GetPtrNumberOfOverwrittenSelected() = iOverwritten;
        pWizard->GetPtrRecoveryFailures()->clear();

        // progress in KB written, so a single big file moves the bar too
        LONGLONG llTotalKB = llTotalBytes / 1024;
        int iRange = (int) (llTotalKB < 1 ? 1 : (llTotalKB > 0x7FFFFFFF ? 0x7FFFFFFF : llTotalKB));

        // app-modal and never hidden automatically: the wizard can't be used until the recovery has finished
        // (or was cancelled) - otherwise a second Retrieve would start a second copy over the same drive
        wxProgressDialog dialog(wxT("Recovering files"),
                wxString("Preparing...") + "\n\n\n\n",
                iRange, // range
                this, // parent
                wxPD_APP_MODAL |
                wxPD_CAN_ABORT |
                wxPD_ELAPSED_TIME |
                wxPD_REMAINING_TIME |
                wxPD_SMOOTH
                );

        MyThreadCopyFiles *thCopyFiles = new MyThreadCopyFiles(this);

        thCopyFiles->SetNTFSDrivePtr(pWizard->GetPtrCNTFSDrive());
        thCopyFiles->SetFinfoHashPtr(pWizard->GetPtrFileInfoHash());
        thCopyFiles->SetFilesToCopy(vFilesToCopy);
        thCopyFiles->SetDestinationDirPtr(pWizard->GetMw_ptrDestinationDir());
        thCopyFiles->SetCancelProgresControl(false);
        thCopyFiles->SetIptrNumberOfCopiedFiles(ptrNumberOfCopiedFiles);

        if (thCopyFiles->Create() != wxTHREAD_NO_ERROR) {
            delete thCopyFiles;
            wxMessageDialog dial(this, languagePack->getWXText(SELECT_DESTINATION_PAGE, CANT_CREATE_THREADS_ERROR),
                    languagePack->getWXText(GENERAL, ERROR), wxOK | wxICON_ERROR);
            dial.ShowModal();
            event.Veto();
            return;
        }

        SetDialogString("Preparing...");
        this->m_NumberOfFileBeingCopied = 0;
        this->mw_bRunDialog = true;

        // the copy thread now owns the drive - no previews until it's done (the timer still fires in the dialog's loop)
        m_previewTimer.Stop();
        m_bCopying = true;
        thCopyFiles->Run();

        bool cont = true; // ovaj se koristi ako je stisnut cancel
        while (this->mw_bRunDialog) {
            // the bar's maximum would end the dialog - keep it just below until the thread is done
            LONG lKBWritten = thCopyFiles->m_lKBWritten;
            int iValue = (lKBWritten < iRange) ? (int) lKBWritten : iRange - 1;
            wxString sMessage = GetDialogString() + "\n" +
                    FormatFileSize((LONGLONG) lKBWritten * 1024) + " of " + FormatFileSize(llTotalBytes);

            // will be set to true if "Skip" button was clicked
            bool skip = false;
            cont = dialog.Update(iValue, sMessage, &skip);

            if (skip) // ovo se ne koristi
                cont = false;

            if (!cont) {
                if (wxMessageBox(languagePack->getWXText(SELECT_DESTINATION_PAGE, DO_YOU_REALLY_WANT_TO_CANCEL),
                        languagePack->getWXText(GENERAL, PROGRESS_DIALOG_QUESTION), // caption
                        wxYES_NO | wxICON_QUESTION, &dialog) == wxYES) {
                    thCopyFiles->SetCancelProgresControl(true); //stop processing in the thread
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
        *pWizard->GetPtrRecoveryFailures() = thCopyFiles->m_vFailures;
        delete thCopyFiles;
        m_bCopying = false;
        wxLog::FlushActive(); // the thread's log lines
        dialog.Hide();

        if (!cont) {
            wxLogStatus(languagePack->getWXText(GENERAL, PROGRESS_DIALOG_ABORTED));
            wxString sMessage;
            sMessage.Printf("Recovery cancelled.\n\n%d of the %d selected files were recovered to\n%s",
                    *ptrNumberOfCopiedFiles, *ptrNumberOfSelectedFiles, *pWizard->GetMw_ptrDestinationDir());
            wxMessageDialog dial(this, sMessage, wxT("Info"), wxOK | wxICON_INFORMATION);
            dial.ShowModal();
            event.Veto(); // stay on the list
        } else {
            wxLogMessage("Recovery finished: %d of %d files.", *ptrNumberOfCopiedFiles, *ptrNumberOfSelectedFiles);
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
    SetDialogString(wxString::Format("Recovering file %d of %d:\n%s", event.GetInt(), *ptrNumberOfSelectedFiles, event.GetString()));
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

// image formats the preview can decode (wxWidgets has no HEIC, WebP, RAW... decoders)

static bool _canPreview(const wchar_t *szFilename) {
    static const char *const PreviewExtensions[] = {"jpg", "jpeg", "jpe", "gif", "png", "bmp", "tif", "tiff", "ico"};
    wxString name(szFilename);
    name.MakeLower(); // -> mala slova
    for (size_t i = 0; i < sizeof (PreviewExtensions) / sizeof (PreviewExtensions[0]); i++)
        if (name.EndsWith(wxString(".") + PreviewExtensions[i]))
            return true;
    return false;
}

// select (or unselect) every listed file of one type, e.g. all images

void wxSelectFilesPage::SelectFilesOfType(int iFileType, bool bSelect) {
    long lCount = m_lcFilesInSelectedDrive->GetFileCount();
    for (long lItem = 0; lItem < lCount; lItem++)
        if (FileTypeOf(m_lcFilesInSelectedDrive->GetFile(lItem)->sFilename.c_str()) == iFileType)
            m_lcFilesInSelectedDrive->SelectItem(lItem, bSelect);
}

void wxSelectFilesPage::OnFilterImagesClick(wxCommandEvent & event) {
    SelectFilesOfType(FILE_TYPE_IMAGE, wxFilterImages->GetValue());
}

void wxSelectFilesPage::OnFilterVideosClick(wxCommandEvent & event) {
    SelectFilesOfType(FILE_TYPE_VIDEO, wxFilterVideo->GetValue());
}

void wxSelectFilesPage::OnFilterMusicClick(wxCommandEvent & event) {
    SelectFilesOfType(FILE_TYPE_MUSIC, wxFilterMusic->GetValue());
}

// the "Show" or "Type" filter changed - files that get hidden also lose their selection (Retrieve takes only what is shown)

void wxSelectFilesPage::OnFilterChoice(wxCommandEvent & event) {
    {
        wxBusyCursor busy; // a few hundred thousand files take a moment
        if (event.GetId() == FILTER_RECOVERABILITY_CHOICE)
            m_lcFilesInSelectedDrive->SetRecoverabilityFilter(m_choiceRecoverability->GetSelection());
        else
            m_lcFilesInSelectedDrive->SetTypeFilter(m_choiceType->GetSelection());
    }
    wxFilterImages->SetValue(false);
    wxFilterVideo->SetValue(false);
    wxFilterMusic->SetValue(false);
    UpdateFoundFilesLabel();
}

// read the start of the selected files (or of every shown file) and compare it with the file type. Runs in the
// GUI thread like the preview - one small read per file, and the progress dialog keeps the window responsive.

void wxSelectFilesPage::OnCheckContentsClick(wxCommandEvent & event) {
    std::vector<t_FileInfo *> vFiles;
    m_lcFilesInSelectedDrive->GetSelectedFiles(vFiles);
    if (vFiles.empty())
        for (long lItem = 0; lItem < m_lcFilesInSelectedDrive->GetFileCount(); lItem++)
            vFiles.push_back(m_lcFilesInSelectedDrive->GetFile(lItem));
    if (vFiles.empty())
        return;

    CNTFSDrive *pDrive = ((MyWizard *) GetParent())->GetPtrCNTFSDrive();
    wxProgressDialog dialog(wxT("Checking contents"), wxString::Format("Checking %d files...", (int) vFiles.size()),
            (int) vFiles.size(), this, wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME | wxPD_AUTO_HIDE);

    int aCount[CONTENT_UNREADABLE + 1] = {0};
    size_t i;
    DWORD dwLastUpdate = 0;
    for (i = 0; i < vFiles.size(); i++) {
        if (GetTickCount() - dwLastUpdate > 100) {
            dwLastUpdate = GetTickCount();
            if (!dialog.Update((int) i, wxString::Format("Checking file %d of %d", (int) i + 1, (int) vFiles.size())))
                break; // cancelled - what was checked so far is kept
        }
        if (vFiles[i]->iContent == CONTENT_NOT_CHECKED)
            pDrive->CheckFileContents(*vFiles[i]);
        if (vFiles[i]->iContent >= 0 && vFiles[i]->iContent <= CONTENT_UNREADABLE)
            aCount[vFiles[i]->iContent]++;
    }
    dialog.Hide();

    m_lcFilesInSelectedDrive->Refilter(); // the "Contents look valid" filter and the sort may depend on it
    UpdateFoundFilesLabel();

    wxString sSummary = wxString::Format("%d files checked%s:\n\n", (int) i, i < vFiles.size() ? " (cancelled)" : "");
    sSummary += wxString::Format("Looks valid: %d\n", aCount[CONTENT_OK]);
    sSummary += wxString::Format("Damaged: %d\n", aCount[CONTENT_DAMAGED] + aCount[CONTENT_ZEROS]);
    sSummary += wxString::Format("Start overwritten: %d\n", aCount[CONTENT_START_OVERWRITTEN]);
    sSummary += wxString::Format("Can't check this file type: %d\n", aCount[CONTENT_NO_SIGNATURE]);
    if (aCount[CONTENT_UNREADABLE])
        sSummary += wxString::Format("Can't read: %d\n", aCount[CONTENT_UNREADABLE]);
    sSummary += "\n\"Looks valid\" means the start of the file matches its type - the rest of it can still be damaged.";
    wxMessageBox(sSummary, wxT("Contents checked"), wxOK | wxICON_INFORMATION, this);
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

    // a short delay, so scrolling through the list with the arrow keys doesn't read every file on the way
    m_previewTimer.Start(250, wxTIMER_ONE_SHOT);
}

// show the focused file in the preview box. Reads the deleted file's data from the drive (in the GUI thread -
// only while no scan or copy thread is using the drive) and tries to decode it as an image.

#define MAX_PREVIEW_FILE_SIZE (32 * 1024 * 1024)

void wxSelectFilesPage::OnPreviewTimer(wxTimerEvent & event) {
    if (m_bCopying)
        return;

    long lItem = m_lcFilesInSelectedDrive->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
    t_FileInfo *pFile = m_lcFilesInSelectedDrive->GetFile(lItem);

    m_bmpPreview->SetBitmap(wxNullBitmap);
    if (!pFile) {
        m_txtPreview->SetLabel(wxT("Select an image\nto see a preview."));
        return;
    }

    // the focused file's contents are checked right away - one small read
    if (pFile->iContent == CONTENT_NOT_CHECKED) {
        ((MyWizard *) GetParent())->GetPtrCNTFSDrive()->CheckFileContents(*pFile);
        m_lcFilesInSelectedDrive->RefreshItem(lItem);
    }
    if (FileTypeOf(pFile->sFilename.c_str()) != FILE_TYPE_IMAGE) {
        m_txtPreview->SetLabel(wxT("No preview -\nnot an image."));
        return;
    }
    if (!_canPreview(pFile->sFilename.c_str())) {
        m_txtPreview->SetLabel(wxT("No preview for\nthis image format."));
        return;
    }
    if (pFile->n64RealSize > MAX_PREVIEW_FILE_SIZE) {
        m_txtPreview->SetLabel(wxT("Too large\nfor a preview."));
        return;
    }

    BYTE *puchData = NULL;
    DWORD dwLen = 0;
    int nRet;
    {
        wxBusyCursor busy;
        nRet = ((MyWizard *) GetParent())->GetPtrCNTFSDrive()->Read_File(pFile->iRecordId, puchData, dwLen);
    }
    if (nRet) {
        m_txtPreview->SetLabel(wxT("No preview -\nthe data can't be read."));
        return;
    }

    wxImage image;
    {
        wxLogNull noLog; // a damaged image is expected here, not an error worth a message
        wxMemoryInputStream stream(puchData, dwLen);
        image.LoadFile(stream, wxBITMAP_TYPE_ANY);
    }
    delete[] puchData;

    if (!image.IsOk()) {
        m_txtPreview->SetLabel(wxT("No preview - the data\nis probably damaged."));
        return;
    }

    wxString sSize = wxString::Format("%d x %d", image.GetWidth(), image.GetHeight());
    double dScale = wxMin((double) PREVIEW_SIZE / image.GetWidth(), (double) PREVIEW_SIZE / image.GetHeight());
    if (dScale < 1.0)
        image.Rescale(wxMax(1, (int) (image.GetWidth() * dScale)), wxMax(1, (int) (image.GetHeight() * dScale)), wxIMAGE_QUALITY_HIGH);
    m_bmpPreview->SetBitmap(wxBitmap(image));
    m_txtPreview->SetLabel(sSize);
}

void wxSelectFilesPage::OnListColumnClick(wxCommandEvent & event) {
    // the same column again reverses the order
    m_lcFilesInSelectedDrive->SortByColumn(((wxListEvent *) & event)->GetColumn());
}

void wxSelectFilesPage::UpdateFoundFilesLabel() {
    ILanguage *languagePackLocal = ((MyWizard *) GetParent())->GetLanguagePack();
    long lShown = m_lcFilesInSelectedDrive->GetFileCount();
    long lTotal = m_lcFilesInSelectedDrive->GetTotalFileCount();
    wxSelectFilesMessage->SetLabel(wxString::Format(languagePackLocal->getWXText(SELECT_FILES_PAGE, SELECT_FILES_OTHER), (int) lTotal));
    m_txtShowing->SetLabel(lShown == lTotal ? wxString::Format("Showing all %ld files", lTotal)
            : wxString::Format("Showing %ld of %ld files", lShown, lTotal));
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
EVT_TIMER(PREVIEW_TIMER, wxSelectFilesPage::OnPreviewTimer)
//EVT_WIZARD_CANCEL(wxID_ANY, wxSelectFilesPage::OnWizardCancel)
END_EVENT_TABLE()




// TODO: Nacin selektovanja fajlova. Da ne mora Ctrl da se drzi