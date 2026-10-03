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

#include "SidePicture3.xpm"

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

    m_lcFilesInSelectedDrive = new wxListCtrl(this, LIST_CTRL_DELETED_FILES, wxPoint(-1, -1),
            wxSize(600, 400), wxLC_REPORT | wxBORDER_THEME | wxLC_HRULES | wxLC_VRULES);
    ((MyWizard *) GetParent())->SetPtrSelectedFiles(m_lcFilesInSelectedDrive);

    wxString filename;
    //    wxString path = m_gdirChooseSourceDrive->GetPath();
    wxString path;
    path.Printf("C:\\"); // sta ako nema d disk?! cemu sluzi ovaj d disk? Da li sme na C disk????
    // ovde bi trebalo proveriti da li postoji D disk i tek onda ga uzeti kao odrediste.

    wxDir dir(path);

    bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);

    int i = 0;

    m_lcFilesInSelectedDrive->ClearAll();

    wxListItem itemCol;
    itemCol.SetText(languagePack->getWXText(SELECT_FILES_PAGE, SELECT_ALL_BUTTON_LABEL));
    itemCol.SetImage(-1);
    itemCol.SetWidth(50);
    m_lcFilesInSelectedDrive->InsertColumn(0, itemCol);

    // to speed up inserting we hide the control temporarily
    m_lcFilesInSelectedDrive->Hide();

    // ovde sad ide ona funkcija koja trpa u listu
    while (cont) {

        long lIndex = m_lcFilesInSelectedDrive->InsertItem(i, filename, 0);
        m_lcFilesInSelectedDrive->SetItemData(lIndex, i);

        filename.Printf(wxT(""));

        cont = dir.GetNext(&filename);
        i += 1;
    }

    m_lcFilesInSelectedDrive->Show();

    m_lcFilesInSelectedDrive->SetColumnWidth(0, wxLIST_AUTOSIZE);
    m_lcFilesInSelectedDrive->SetColumnWidth(1, wxLIST_AUTOSIZE);
    m_lcFilesInSelectedDrive->SetColumnWidth(2, wxLIST_AUTOSIZE);


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

        *ptrNumberOfSelectedFiles = m_lcFilesInSelectedDrive->GetSelectedItemCount();
        //        (*this->m_NumberOfSelectedFiles) = ;//sto je ovo glupo resenje - katastrofa - ne treba mu jos jedna referenca na isti pointer

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
        thCopyFiles->SetListControlPtr(((MyWizard *) GetParent())->GetPtrSelectedFiles());
        thCopyFiles->SetDestinationDirPtr(((MyWizard *) GetParent())->GetMw_ptrDestinationDir());
        thCopyFiles->SetMutexLockPtr(((MyWizard *) GetParent())->GetPtrListMutex()); // ovo nicemu ne sluzi
        thCopyFiles->SetCancelProgresControl(false);
        thCopyFiles->SetMwPtr_strSelectedDir(((MyWizard *) GetParent())->GetMw_ptrDestinationDir());
        thCopyFiles->SetIptrNumberOfCopiedFiles(((MyWizard *) GetParent())->GetIptrNumberOfCopiedFiles());

        if (thCopyFiles->Create() != wxTHREAD_NO_ERROR) {
            wxLogError(languagePack->getWXText(SELECT_DESTINATION_PAGE, CANT_CREATE_THREADS_ERROR));
            exit(-1);
        }

        thCopyFiles->Run();

        bool cont = true; // ovaj se koristi ako je stisnut cancel - to moram da vidim gde sta treba da implementiram... 
        // moguce da cu na ovoj strani da stavim cancel a na sledecoj oba, i skip i cancel hmmm...         
        this->mw_bRunDialog = true;
        this->mw_sDialogString = "Copying deleted files...";
        this->m_NumberOfFileBeingCopied = 0;
        while (this->mw_bRunDialog) {
            // will be set to true if "Skip" button was clicked
            bool skip = false;
            cont = dialog.Update(this->m_NumberOfFileBeingCopied, this->mw_sDialogString, &skip);

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

void wxSelectFilesPage::OnPulseDialogEvent(wxCommandEvent & event) {
    int *ptrNumberOfSelectedFiles = ((MyWizard *) GetParent())->GetMw_ptriNumberOfSelectedFiles();
    this->mw_sDialogString.Printf("Copying %d/%d\nFilename: %s", event.GetInt(), *ptrNumberOfSelectedFiles, event.GetString());
    this->m_NumberOfFileBeingCopied = event.GetInt();
}

void wxSelectFilesPage::OnSelectAllClick(wxCommandEvent & event) {
    wxListCtrl *tmpFileList = ((MyWizard *) GetParent())->GetPtrSelectedFiles();

    int itemNumber = tmpFileList->GetItemCount();

    for (int item = 0; item < itemNumber; item++) {
        tmpFileList->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        //tmpFileList->SetItemState(item, wxLIST_STATE_FOCUSED, wxLIST_STATE_SELECTED);

    }
    wxFilterImages->SetValue(true);
    wxFilterVideo->SetValue(true);
    wxFilterMusic->SetValue(true);

}

void wxSelectFilesPage::OnUnselectAllClick(wxCommandEvent & event) {
    wxListCtrl *tmpFileList = ((MyWizard *) GetParent())->GetPtrSelectedFiles();

    int itemNumber = tmpFileList->GetItemCount();

    for (int item = 0; item < itemNumber; item++) {
        tmpFileList->SetItemState(item, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    }

    wxFilterImages->SetValue(false);
    wxFilterVideo->SetValue(false);
    wxFilterMusic->SetValue(false);
}

bool _isImage(wxString *cell_contents_string) {
    char ImageExtensions[][4] = {"jpg", "gif", "png", "bmp"}; //sta raditi sa ekstenzijom jpeg? 
    int size = sizeof (ImageExtensions) / sizeof (ImageExtensions[0]);

    cell_contents_string->MakeLower(); // -> mala slova
    for (int i = 0; i < size; i++) {
        if (cell_contents_string->EndsWith(wxString::FromAscii(ImageExtensions[i]), NULL))
            return true;
    }

    return false;
}

void wxSelectFilesPage::OnFilterImagesClick(wxCommandEvent & event) {
    wxListCtrl *tmpFileList = ((MyWizard *) GetParent())->GetPtrSelectedFiles();
    wxListItem row_info;
    wxString cell_contents_string;
    int itemNumber = tmpFileList->GetItemCount();
    int column = FILE_NAME_COLUMN;

    for (int item = 0; item < itemNumber; item++) {
        // Set what row it is (m_itemId is a member of the regular wxListCtrl class)
        row_info.m_itemId = item;
        // Set what column of that row we want to query for information.
        row_info.m_col = column;
        // Set text mask
        row_info.m_mask = wxLIST_MASK_TEXT;
        // Get the info and store it in row_info variable.   
        tmpFileList->GetItem(row_info);
        // Extract the text out that cell
        cell_contents_string = row_info.m_text;

        if (_isImage(&cell_contents_string)) {
            if (wxFilterImages->GetValue())// da li je dugme uklj. ili isklj.?
                tmpFileList->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            else
                tmpFileList->SetItemState(item, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
        }
    }
}

bool _isVideo(wxString *cell_contents_string) {

    char VideoExtensions[][4] = {"avi", "mov", "wmv", "mp4", "mkv"}; //ovo bi mozda trebalo da ide u header file, da bi se lakse konfigurisalo. 
    int size = sizeof (VideoExtensions) / sizeof (VideoExtensions[0]);

    cell_contents_string->MakeLower();
    for (int i = 0; i < size; i++) {
        if (cell_contents_string->EndsWith(wxString::FromAscii(VideoExtensions[i]), NULL))
            return true;
    }

    return false;
}

void wxSelectFilesPage::OnFilterVideosClick(wxCommandEvent & event) {
    wxListCtrl *tmpFileList = ((MyWizard *) GetParent())->GetPtrSelectedFiles();
    wxListItem row_info;
    wxString cell_contents_string;
    int itemNumber = tmpFileList->GetItemCount();
    int column = FILE_NAME_COLUMN; //TODO ovo ne sme da bude static kompajlirano, ovo je kolona u kojoj je tekst

    for (int item = 0; item < itemNumber; item++) {
        // Set what row it is (m_itemId is a member of the regular wxListCtrl class)
        row_info.m_itemId = item;
        // Set what column of that row we want to query for information.
        row_info.m_col = column;
        // Set text mask
        row_info.m_mask = wxLIST_MASK_TEXT;
        // Get the info and store it in row_info variable.   
        tmpFileList->GetItem(row_info);
        // Extract the text out that cell
        cell_contents_string = row_info.m_text;

        if (_isVideo(&cell_contents_string)) {
            if (wxFilterVideo->GetValue())// da li je dugme stisnuto? vidi kako se ovo tacno radi
                tmpFileList->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            else
                tmpFileList->SetItemState(item, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
        }
    }

}

bool _isMusic(wxString *cell_contents_string) {

    char MusicExtensions[][4] = {"mp3", "wma"}; //ovo bi mozda trebalo da ide u header file, da bi se lakse konfigurisalo. 
    int size = sizeof (MusicExtensions) / sizeof (MusicExtensions[0]);

    cell_contents_string->MakeLower();
    for (int i = 0; i < size; i++) {
        if (cell_contents_string->EndsWith(wxString::FromAscii(MusicExtensions[i]), NULL))
            return true;
    }

    return false;
}

void wxSelectFilesPage::OnFilterMusicClick(wxCommandEvent & event) {
    wxListCtrl *tmpFileList = ((MyWizard *) GetParent())->GetPtrSelectedFiles();
    wxListItem row_info;
    wxString cell_contents_string;
    int itemNumber = tmpFileList->GetItemCount();
    int column = FILE_NAME_COLUMN;

    for (int item = 0; item < itemNumber; item++) {
        // Set what row it is (m_itemId is a member of the regular wxListCtrl class)
        row_info.m_itemId = item;
        // Set what column of that row we want to query for information.
        row_info.m_col = column;
        // Set text mask
        row_info.m_mask = wxLIST_MASK_TEXT;
        // Get the info and store it in row_info variable.   
        tmpFileList->GetItem(row_info);
        // Extract the text out that cell
        cell_contents_string = row_info.m_text;

        if (_isMusic(&cell_contents_string)) {
            if (wxFilterVideo->GetValue())// da li je dugme stisnuto? vidi kako se ovo tacno radi
                tmpFileList->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            else
                tmpFileList->SetItemState(item, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
        }
    }

}

void wxSelectFilesPage::OnSearchAction(wxCommandEvent & event) {

    FileInfoHash *m_finfohash = ((MyWizard *) GetParent())->GetPtrFileInfoHash();
    wxListCtrl *m_ListControl = ((MyWizard *) GetParent())->GetPtrSelectedFiles();
    wxString search_item = event.GetString();

    if (m_finfohash->empty()) {
        wxString myErrorMessage;
        myErrorMessage.Printf("There were no deleted files  found on this drive.");

        wxMessageDialog *dial = new wxMessageDialog(NULL,
                myErrorMessage, wxT("Info"), wxOK | wxICON_ERROR);
        dial->ShowModal();
    }

    //    strcpy(sCompareString, m_wxSearchCtrlSearchList->GetLabelText();

    m_ListControl->ClearAll();
    m_ListControl->InsertColumn(0, wxT("File name"), wxLIST_FORMAT_LEFT, -1);
    m_ListControl->InsertColumn(1, wxT("Status"), wxLIST_FORMAT_LEFT, -1);
    // to speed up inserting we hide the control temporarily
    //    m_ListControl->Hide();

    // ovde sad ide ona funkcija koja trpa u listu
    int listCounter = 30;
    FileInfoHash::iterator it;
    t_FileInfo *pTempFinfo = NULL;
    for (it = m_finfohash->begin(); it != m_finfohash->end(); ++it) {
        pTempFinfo = it->second;
        //ovde se vrsi compare, za prazan string ubaci sve elemente hash tabele
        if (search_item.IsEmpty() || wxString(pTempFinfo->szFilename).Upper().Contains(search_item.Upper())) {
            long lIndex = m_ListControl->InsertItem(listCounter, pTempFinfo->szFilename, 0);
            m_ListControl->SetItemPtrData(lIndex, (wxUIntPtr) pTempFinfo);
            m_ListControl->SetItem(lIndex, 1, pTempFinfo->szBuffer);
            listCounter++;
        }

    }

    m_ListControl->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    m_ListControl->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
    //    m_ListControl->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER); // bilo je wxLIST_AUTOSIZE
    m_ListControl->RefreshItem(0);
    m_ListControl->Show();
    //process the event?
}

void wxSelectFilesPage::OnSearchCancel(wxCommandEvent & event) {
    FileInfoHash *m_finfohash = ((MyWizard *) GetParent())->GetPtrFileInfoHash();
    wxListCtrl *m_ListControl = ((MyWizard *) GetParent())->GetPtrSelectedFiles();

    if (m_finfohash->empty()) {
        wxString myErrorMessage;
        myErrorMessage.Printf("There were no deleted files  found on this drive.");

        wxMessageDialog *dial = new wxMessageDialog(NULL,
                myErrorMessage, wxT("Info"), wxOK | wxICON_ERROR);
        dial->ShowModal();
    }

    m_ListControl->ClearAll();
    m_ListControl->InsertColumn(0, wxT("File name"), wxLIST_FORMAT_LEFT, -1);
    m_ListControl->InsertColumn(1, wxT("Status"), wxLIST_FORMAT_LEFT, -1);
    // to speed up inserting we hide the control temporarily
    m_ListControl->Hide();

    // ovde sad ide ona funkcija koja trpa u listu bez pitanja
    int listCounter = 30;
    FileInfoHash::iterator it;
    t_FileInfo *pTempFinfo = NULL;
    for (it = m_finfohash->begin(); it != m_finfohash->end(); ++it) {
        pTempFinfo = it->second;
        long lIndex = m_ListControl->InsertItem(listCounter, pTempFinfo->szFilename, 0);
        m_ListControl->SetItemPtrData(lIndex, (wxUIntPtr) pTempFinfo);
        m_ListControl->SetItem(lIndex, 1, pTempFinfo->szBuffer);
        listCounter++;
    }

    m_ListControl->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    m_ListControl->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
    //    m_ListControl->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER); // bilo je wxLIST_AUTOSIZE
    m_ListControl->RefreshItem(0);
    m_ListControl->Show();

}

void wxSelectFilesPage::OnFilesListClick(wxCommandEvent & event) {
    wxFilterImages->SetValue(false);
    wxFilterVideo->SetValue(false);
    wxFilterMusic->SetValue(false);
    m_lcFilesInSelectedDrive->SetItemState(1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
}

int wxCALLBACK
MyCompareFunctionColumn0Ascending(wxIntPtr item1, wxIntPtr item2, wxIntPtr WXUNUSED(sortData)) {
    t_FileInfo *pTempItem1, *pTempItem2;
    pTempItem1 = (t_FileInfo *) item1;
    pTempItem2 = (t_FileInfo *) item2;

    return (strcmp(pTempItem1->szFilename, pTempItem2->szFilename));
}

int wxCALLBACK
MyCompareFunctionColumn1Ascending(wxIntPtr item1, wxIntPtr item2, wxIntPtr WXUNUSED(sortData)) {
    t_FileInfo *pTempItem1, *pTempItem2;
    pTempItem1 = (t_FileInfo *) item1;
    pTempItem2 = (t_FileInfo *) item2;

    return pTempItem1->n64SizeOnDisk - pTempItem2->n64SizeOnDisk;
}

int wxCALLBACK
MyCompareFunctionColumn2Ascending(wxIntPtr item1, wxIntPtr item2, wxIntPtr WXUNUSED(sortData)) {
    t_FileInfo *pTempItem1, *pTempItem2;
    pTempItem1 = (t_FileInfo *) item1;
    pTempItem2 = (t_FileInfo *) item2;

    return (strcmp(pTempItem1->szBuffer, pTempItem2->szBuffer));
}

int wxCALLBACK
MyCompareFunctionColumn0Descending(wxIntPtr item1, wxIntPtr item2, wxIntPtr WXUNUSED(sortData)) {
    t_FileInfo *pTempItem1, *pTempItem2;
    pTempItem1 = (t_FileInfo *) item1;
    pTempItem2 = (t_FileInfo *) item2;

    return (strcmp(pTempItem2->szFilename, pTempItem1->szFilename));
}

int wxCALLBACK
MyCompareFunctionColumn1Descending(wxIntPtr item1, wxIntPtr item2, wxIntPtr WXUNUSED(sortData)) {
    t_FileInfo *pTempItem1, *pTempItem2;
    pTempItem1 = (t_FileInfo *) item1;
    pTempItem2 = (t_FileInfo *) item2;

    return pTempItem2->n64SizeOnDisk - pTempItem1->n64SizeOnDisk;
}

int wxCALLBACK
MyCompareFunctionColumn2Descending(wxIntPtr item1, wxIntPtr item2, wxIntPtr WXUNUSED(sortData)) {
    t_FileInfo *pTempItem1, *pTempItem2;
    pTempItem1 = (t_FileInfo *) item1;
    pTempItem2 = (t_FileInfo *) item2;

    return (strcmp(pTempItem2->szBuffer, pTempItem1->szBuffer));
}

void wxSelectFilesPage::OnListColumnClick(wxCommandEvent & event) {
    static bool bColumn0NextSortIsAscending = true; // sa ovim postizemo da kad klikces na column header on okrece sort iz ASC u DESC
    static bool bColumn1NextSortIsAscending = true; // sa ovim postizemo da kad klikces na column header on okrece sort iz ASC u DESC
    static bool bColumn2NextSortIsAscending = true; // sa ovim postizemo da kad klikces na column header on okrece sort iz ASC u DESC
    
    int col = ((wxListEvent *) & event)->m_col;
    switch (col) {
        case 0:
            if (bColumn0NextSortIsAscending) {
                bColumn0NextSortIsAscending = false;
                m_lcFilesInSelectedDrive->SortItems(MyCompareFunctionColumn0Ascending, 0);
            } else {
                m_lcFilesInSelectedDrive->SortItems(MyCompareFunctionColumn0Descending, 0);
                bColumn0NextSortIsAscending = true;
            }
            break;
        
        case 1:
            if (bColumn1NextSortIsAscending) {
                bColumn1NextSortIsAscending = false;
                m_lcFilesInSelectedDrive->SortItems(MyCompareFunctionColumn1Ascending, 0);
            } else {
                m_lcFilesInSelectedDrive->SortItems(MyCompareFunctionColumn1Descending, 0);
                bColumn1NextSortIsAscending = true;
            }
            break;


        case 2:
            if (bColumn2NextSortIsAscending) {
                bColumn2NextSortIsAscending = false;
                m_lcFilesInSelectedDrive->SortItems(MyCompareFunctionColumn2Ascending, 0);
            } else {
                m_lcFilesInSelectedDrive->SortItems(MyCompareFunctionColumn2Descending, 0);
                bColumn2NextSortIsAscending = true;
            }
            break;
    }

    return;
}

void wxSelectFilesPage::OnPageShown(wxWizardEvent& event) {

    ILanguage *languagePackLocal = ((MyWizard *) GetParent())->GetLanguagePack();
    wxWindow* NextButton = FindWindowById(wxID_FORWARD, GetParent());
    NextButton->SetLabel(languagePackLocal->getWXText(GENERAL, RETRIEVE_BUTTON_LABEL));

    wxListCtrl *tmpFileList = ((MyWizard *) GetParent())->GetPtrSelectedFiles();
    int itemNumber = tmpFileList->GetItemCount();

    wxString wxsLabel; //zoran SELECT_FILES_OTHER
    wxsLabel.Printf("Found %d files.\n"
            "Select the files you want to try to retrieve and click \'Retrieve\'\n\n"
            "To select multiple files, hold Ctrl.", itemNumber
            );

    //m_wxEndPageText = new wxStaticText(this, -1,              wxString::Format(languagePackLocal->getWXText(SELECT_FILES_PAGE, SELECT_FILES_OTHER),itemNumber));
    wxSelectFilesMessage->SetLabel(wxString::Format(languagePackLocal->getWXText(SELECT_FILES_PAGE, SELECT_FILES_OTHER), itemNumber));

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