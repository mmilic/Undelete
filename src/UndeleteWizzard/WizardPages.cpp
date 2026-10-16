// WizardPages.cpp: the four pages of the wizard - source drive, destination, files, done

#include <windows.h>
#include <shellapi.h> // ShellExecute - not in a WIN32_LEAN_AND_MEAN windows.h
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/filename.h>
#include <wx/progdlg.h>

#include "WizardFrame.h"
#include "DeletedFilesList.h"
#include "FileDetailsPanel.h"
#include "FileContents.h"
#include "Theme.h"
#include "Volumes.h"
#include "languages/Translate.h"

static wxStaticText *Label(wxWindow *parent, const wxString &sText, const wxFont &font, const wxColour &colour) {
    wxStaticText *pText = new wxStaticText(parent, wxID_ANY, sText);
    pText->SetFont(font);
    pText->SetForegroundColour(colour);
    return pText;
}

/////////////////////////////////// step 1: source drive ///////////////////////////////////

CSourcePage::CSourcePage(wxWindow *parent, CWizardFrame *pFrame, const std::vector<ST_DRIVE> &vDrives)
: wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL) {
    m_pFrame = pFrame;
    SetBackgroundColour(Theme::PanelBg);
    SetScrollRate(0, FromDIP(10));

    wxFlexGridSizer *pGrid = new wxFlexGridSizer(2, FromDIP(12), FromDIP(12));
    pGrid->AddGrowableCol(0, 1);
    pGrid->AddGrowableCol(1, 1);
    for (size_t i = 0; i < vDrives.size(); i++) {
        CDriveCard *pCard = new CDriveCard(this, wxID_ANY, vDrives[i], false);
        pCard->Bind(wxEVT_BUTTON, &CSourcePage::OnCard, this);
        pGrid->Add(pCard, 1, wxEXPAND);
        m_vCards.push_back(pCard);
    }

    wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
    if (vDrives.empty())
        pSizer->Add(Label(this, Tr(L"No hard disks or USB drives were found."), Theme::Body(), Theme::TextSecondary), 0, wxALL, FromDIP(4));
    pSizer->Add(pGrid, 0, wxEXPAND | wxRIGHT, FromDIP(4)); // room for the scroll bar
    SetSizer(pSizer);
}

void CSourcePage::OnCard(wxCommandEvent &event) {
    CDriveCard *pCard = (CDriveCard *) event.GetEventObject();
    if (pCard->GetDrive().bSupported)
        m_pFrame->SetSource(pCard->GetDrive());
}

void CSourcePage::ShowSelection(const wxString &sSource) {
    for (size_t i = 0; i < m_vCards.size(); i++)
        m_vCards[i]->SetSelected(m_vCards[i]->GetDrive().sSource == sSource);
}

/////////////////////////////////// step 2: destination ///////////////////////////////////

CDestinationPage::CDestinationPage(wxWindow *parent, CWizardFrame *pFrame)
: wxPanel(parent, wxID_ANY) {
    m_pFrame = pFrame;
    SetBackgroundColour(Theme::PanelBg);

    wxStaticText *pLabel = Label(this, Tr(L"Save recovered files to"), Theme::Bold(Theme::Body()), Theme::Text);
    m_pPath = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    m_pPath->SetFont(Theme::Body());
    m_pPath->SetMinSize(FromDIP(wxSize(520, -1))); // the column's width must not depend on the suggestions
    m_pPath->SetHint(Tr(L"A folder on another drive"));
    CFlatButton *pBrowse = new CFlatButton(this, wxID_ANY, Tr(L"Browse..."), FLAT_BUTTON_SECONDARY);
    pBrowse->Bind(wxEVT_BUTTON, &CDestinationPage::OnBrowse, this);
    m_pPath->Bind(wxEVT_TEXT, [this](wxCommandEvent &) { CheckDestination(); });

    wxBoxSizer *pRow = new wxBoxSizer(wxHORIZONTAL);
    pRow->Add(m_pPath, 1, wxALIGN_CENTER_VERTICAL);
    pRow->Add(pBrowse, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(8));

    m_pChipsLabel = Label(this, Tr(L"Suggestions:"), Theme::Small(), Theme::TextSecondary);
    m_pChips = new wxWrapSizer(wxHORIZONTAL, wxREMOVE_LEADING_SPACES); // chips as wide as their text
    m_pChips->Add(m_pChipsLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, FromDIP(8));

    m_pWarning = new CBanner(this, BANNER_WARNING);
    m_pOk = new CBanner(this, BANNER_INFO);

    wxBoxSizer *pColumn = new wxBoxSizer(wxVERTICAL);
    pColumn->Add(pLabel, 0, wxBOTTOM, FromDIP(8));
    pColumn->Add(pRow, 0, wxEXPAND | wxBOTTOM, FromDIP(12));
    pColumn->Add(m_pChips, 0, wxEXPAND | wxBOTTOM, FromDIP(4));
    pColumn->Add(m_pWarning, 0, wxEXPAND);
    pColumn->Add(m_pOk, 0, wxEXPAND);
    pColumn->SetMinSize(FromDIP(wxSize(680, -1)));

    wxBoxSizer *pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(pColumn, 0);
    pSizer->AddStretchSpacer();
    SetSizer(pSizer);
    m_pWarning->Hide();
    m_pOk->Hide();
}

void CDestinationPage::OnShow() {
    const ST_DRIVE *pSource = m_pFrame->GetSource();
    if (!pSource)
        return;

    // the suggestions follow the source drive
    while (m_pChips->GetItemCount() > 1) {
        wxWindow *pChip = m_pChips->GetItem(1)->GetWindow();
        m_pChips->Detach(1);
        pChip->Destroy();
    }
    std::vector<wxString> vSuggestions = SuggestDestinations(pSource->sSource, 3);
    for (size_t i = 0; i < vSuggestions.size(); i++) {
        CFlatButton *pChip = new CFlatButton(this, wxID_ANY, vSuggestions[i], FLAT_BUTTON_CHIP);
        pChip->Bind(wxEVT_BUTTON, &CDestinationPage::OnChip, this);
        m_pChips->Add(pChip, 0, wxRIGHT | wxBOTTOM, FromDIP(8));
    }
    m_pChipsLabel->Show(!vSuggestions.empty());

    // a folder the user chose stays; one this page filled in follows the source
    if (m_pPath->GetValue() == m_sSuggested) {
        m_sSuggested = vSuggestions.empty() ? wxString() : vSuggestions[0];
        m_pPath->ChangeValue(m_sSuggested);
    }
    CheckDestination();
    Layout();
}

wxString CDestinationPage::GetDestination() const {
    wxString sPath = m_pPath->GetValue();
    sPath.Trim().Trim(false);
    return sPath;
}

bool CDestinationPage::IsValid() const {
    const ST_DRIVE *pSource = m_pFrame->GetSource();
    wxString sPath = GetDestination();
    if (!pSource || sPath.IsEmpty() || !wxIsAbsolutePath(sPath))
        return false;
    return !IsOnSameVolume(sPath, pSource->sSource);
}

void CDestinationPage::CheckDestination() {
    const ST_DRIVE *pSource = m_pFrame->GetSource();
    wxString sPath = GetDestination();
    bool bSameDrive = pSource && !sPath.IsEmpty() && IsOnSameVolume(sPath, pSource->sSource);
    bool bOk = !bSameDrive && IsValid();
    wxString sDrive = pSource ? pSource->sName : wxString();

    if (bSameDrive)
        m_pWarning->SetMessage(wxString::Format(Tr(L"This folder is on %s, the drive you are recovering from."), sDrive),
            Tr(L"Writing there can overwrite the very files you want back. Pick a folder on another drive."));
    if (bOk)
        m_pOk->SetMessage(wxEmptyString, pSource->bImage
            ? wxString(Tr(L"Next, the disk image is scanned - nothing is written to it."))
            : wxString::Format(Tr(L"Good choice: a different drive than %s. Next, the drive is scanned - nothing is written to it."), sDrive));
    m_pWarning->Show(bSameDrive);
    m_pOk->Show(bOk);
    Layout();
    m_pFrame->UpdateFooter();
}

void CDestinationPage::OnBrowse(wxCommandEvent &WXUNUSED(event)) {
    wxDirDialog dialog(this, Tr(L"Choose a folder on another drive"), GetDestination());
    if (dialog.ShowModal() != wxID_OK)
        return;
    wxFileName folder = wxFileName::DirName(dialog.GetPath());
    folder.AppendDir("RetrievedFiles"); // the files go into a folder of their own, not loose among others
    m_pPath->SetValue(folder.GetPath());
}

void CDestinationPage::OnChip(wxCommandEvent &event) {
    m_pPath->SetValue(((CFlatButton *) event.GetEventObject())->GetLabel());
}

/////////////////////////////////// step 3: files ///////////////////////////////////

// the grey box with the progress of the scan or the recovery

class CProgressBox : public wxPanel {
public:
    CProgressBox(wxWindow *parent) : wxPanel(parent, wxID_ANY) {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetBackgroundColour(Theme::SubtleBg);
        m_pTitle = Label(this, wxEmptyString, Theme::Bold(Theme::Body()), Theme::Text);
        m_pPercent = Label(this, wxEmptyString, Theme::Body(), Theme::Text);
        m_pBar = new CThinBar(this);
        m_pNote = Label(this, wxEmptyString, Theme::Small(), Theme::TextSecondary);

        wxBoxSizer *pTop = new wxBoxSizer(wxHORIZONTAL);
        pTop->Add(m_pTitle, 1);
        pTop->Add(m_pPercent, 0);
        wxBoxSizer *pInner = new wxBoxSizer(wxVERTICAL);
        pInner->Add(pTop, 0, wxEXPAND);
        pInner->Add(m_pBar, 0, wxEXPAND | wxTOP, FromDIP(10));
        pInner->Add(m_pNote, 0, wxEXPAND | wxTOP, FromDIP(10));
        wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
        pSizer->Add(pInner, 0, wxEXPAND | wxALL, FromDIP(20));
        SetSizer(pSizer);
        Bind(wxEVT_PAINT, [this](wxPaintEvent &) {
            wxAutoBufferedPaintDC dcPaint(this);
            dcPaint.SetBackground(wxBrush(GetParent()->GetBackgroundColour()));
            dcPaint.Clear();
            wxGCDC dc(dcPaint);
            Theme::DrawRoundedBox(dc, GetClientRect(), FromDIP(8), Theme::SubtleBg, wxNullColour);
        });
    }

    void Set(const wxString &sTitle, const wxString &sNote) {
        m_pTitle->SetLabel(sTitle);
        m_pNote->SetLabel(sNote);
        m_pPercent->SetLabel(wxEmptyString);
        m_pBar->SetValue(-1);
        Layout();
    }

    void SetProgress(int iPercent, const wxString &sNote) {
        m_pPercent->SetLabel(iPercent >= 0 ? wxString::Format("%d %%", iPercent) : wxString());
        m_pBar->SetValue(iPercent);
        if (!sNote.IsEmpty() && sNote != m_pNote->GetLabel()) {
            m_pNote->SetLabel(sNote);
            Layout();
            GetParent()->Layout();
        }
    }

private:
    wxStaticText *m_pTitle, *m_pPercent, *m_pNote;
    CThinBar *m_pBar;
};

// a white box with a thin rounded outline around the list

class CListBox : public wxPanel {
public:
    CListBox(wxWindow *parent) : wxPanel(parent, wxID_ANY) {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetBackgroundColour(Theme::PanelBg);
        Bind(wxEVT_PAINT, [this](wxPaintEvent &) {
            wxAutoBufferedPaintDC dcPaint(this);
            dcPaint.SetBackground(wxBrush(GetParent()->GetBackgroundColour()));
            dcPaint.Clear();
            wxGCDC dc(dcPaint);
            Theme::DrawRoundedBox(dc, GetClientRect(), FromDIP(6), Theme::PanelBg, Theme::Border);
        });
    }
};

CFilesPage::CFilesPage(wxWindow *parent, CWizardFrame *pFrame)
: wxPanel(parent, wxID_ANY) {
    m_pFrame = pFrame;
    m_pDrive = NULL;
    SetBackgroundColour(Theme::PanelBg);

    // the progress of the scan or the recovery
    m_pProgressView = new wxPanel(this, wxID_ANY);
    m_pProgressView->SetBackgroundColour(Theme::PanelBg);
    m_pProgress = new CProgressBox(m_pProgressView);
    wxBoxSizer *pProgressColumn = new wxBoxSizer(wxVERTICAL);
    pProgressColumn->Add(m_pProgress, 0, wxEXPAND);
    pProgressColumn->SetMinSize(FromDIP(wxSize(680, -1)));
    wxBoxSizer *pProgressSizer = new wxBoxSizer(wxHORIZONTAL);
    pProgressSizer->Add(pProgressColumn, 0);
    m_pProgressView->SetSizer(pProgressSizer);

    // the list, its filters and the details pane
    m_pListView = new wxPanel(this, wxID_ANY);
    m_pListView->SetBackgroundColour(Theme::PanelBg);
    wxWindow *pView = m_pListView;

    m_pSearch = new wxSearchCtrl(pView, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(240, -1)));
    m_pSearch->SetFont(Theme::Small());
    m_pSearch->SetDescriptiveText(Tr(L"Search name or folder"));
    m_pSearch->ShowCancelButton(true);
    std::vector<wxString> vShow;
    for (int i = RECOVERABILITY_FILTER_ALL; i <= RECOVERABILITY_FILTER_NOT_OVERWRITTEN; i++)
        vShow.push_back(RecoverabilityFilterLabel(i));
    m_pShow = new CSegmented(pView, wxID_ANY, vShow);
    m_pType = new wxChoice(pView, wxID_ANY);
    m_pType->SetFont(Theme::Small());
    for (int i = FILE_TYPE_ANY; i < FILE_TYPE_COUNT; i++) // item index == FILE_TYPE_*
        m_pType->Append(FileTypeFilterLabel(i));
    m_pType->SetSelection(FILE_TYPE_ANY);
    m_pCheckContents = new CFlatButton(pView, wxID_ANY, Tr(L"Check contents"), FLAT_BUTTON_SECONDARY);
    m_pCheckContents->SetToolTip(Tr(L"Reads the start of each ticked file (or of every shown file, if none is ticked)\n"
            L"and checks that it still looks like its type - e.g. that a .jpg still holds a JPEG image."));

    wxBoxSizer *pToolbar = new wxBoxSizer(wxHORIZONTAL);
    pToolbar->Add(m_pSearch, 0, wxALIGN_CENTER_VERTICAL);
    pToolbar->Add(m_pShow, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(8));
    pToolbar->Add(Label(pView, Tr(L"Type"), Theme::Small(), Theme::TextSecondary), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(12));
    pToolbar->Add(m_pType, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(6));

    CListBox *pListBox = new CListBox(pView);
    m_pList = new CDeletedFilesList(pListBox, wxID_ANY);
    wxBoxSizer *pListBoxSizer = new wxBoxSizer(wxVERTICAL);
    pListBoxSizer->Add(m_pList, 1, wxEXPAND | wxALL, FromDIP(2));
    pListBox->SetSizer(pListBoxSizer);

    m_pSummary = Label(pView, wxEmptyString, Theme::Small(), Theme::TextSecondary);
    m_pDetails = new CFileDetailsPanel(pView, 240);
    m_pDetails->OnContentChecked = [this](t_FileInfo *pFile) { m_pList->RefreshFile(pFile); };

    // under the list: how many are shown and ticked, and the content check of what is ticked or shown
    wxBoxSizer *pBelow = new wxBoxSizer(wxHORIZONTAL);
    pBelow->Add(m_pSummary, 1, wxALIGN_CENTER_VERTICAL);
    pBelow->Add(m_pCheckContents, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(8));

    wxBoxSizer *pListColumn = new wxBoxSizer(wxVERTICAL);
    pListColumn->Add(pToolbar, 0, wxEXPAND | wxBOTTOM, FromDIP(10));
    pListColumn->Add(pListBox, 1, wxEXPAND);
    pListColumn->Add(pBelow, 0, wxEXPAND | wxTOP, FromDIP(8));
    wxBoxSizer *pListSizer = new wxBoxSizer(wxHORIZONTAL);
    pListSizer->Add(pListColumn, 1, wxEXPAND);
    pListSizer->Add(m_pDetails, 0, wxLEFT, FromDIP(16));
    pView->SetSizer(pListSizer);

    wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
    pSizer->Add(m_pProgressView, 0, wxEXPAND);
    pSizer->Add(m_pListView, 1, wxEXPAND);
    SetSizer(pSizer);
    m_pListView->Hide();

    m_searchTimer.SetOwner(this);
    m_pSearch->Bind(wxEVT_TEXT, &CFilesPage::OnSearchText, this);
    m_pSearch->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, [this](wxCommandEvent &) { m_pSearch->Clear(); });
    Bind(wxEVT_TIMER, &CFilesPage::OnSearchTimer, this);
    m_pShow->Bind(wxEVT_CHOICE, &CFilesPage::OnShowFilter, this);
    m_pType->Bind(wxEVT_CHOICE, &CFilesPage::OnTypeFilter, this);
    m_pCheckContents->Bind(wxEVT_BUTTON, &CFilesPage::OnCheckContents, this);
    m_pList->Bind(EVT_DELETED_FILES_CHECKED, &CFilesPage::OnChecked, this);
    m_pList->Bind(wxEVT_LIST_ITEM_FOCUSED, &CFilesPage::OnFocused, this);
}

void CFilesPage::ShowProgress(const wxString &sTitle, const wxString &sNote) {
    m_pProgress->Set(sTitle, sNote);
    m_pListView->Hide();
    m_pProgressView->Show();
    Layout();
}

void CFilesPage::SetProgress(int iPercent, const wxString &sNote) {
    m_pProgress->SetProgress(iPercent, sNote);
}

void CFilesPage::ShowList(FileInfoHash *pHash) {
    m_pSearch->ChangeValue(wxEmptyString);
    m_pList->SetNameFilter(wxEmptyString);
    m_pList->SetFiles(pHash);
    ShowListAgain();
}

void CFilesPage::ShowListAgain() {
    m_pProgressView->Hide();
    m_pListView->Show();
    Layout();
    m_pDetails->ShowFile(m_pList->GetFocusedFile(), m_pDrive);
    UpdateSummary();
}

void CFilesPage::ClearList() {
    m_pDetails->ShowFile(NULL, NULL);
    m_pList->Clear();
}

void CFilesPage::SetDriveForPreview(CRecoveryDrive *pDrive) {
    m_pDrive = pDrive;
    m_pCheckContents->Enable(pDrive != NULL);
    m_pDetails->ShowFile(m_pDetails->GetFile(), pDrive);
}

void CFilesPage::UpdateSummary() {
    long lShown = m_pList->GetFileCount(), lTotal = m_pList->GetTotalFileCount();
    int iChecked = m_pList->GetCheckedCount();
    m_pSummary->SetLabel(wxString::Format(Tr(L"Showing %ld of %ld deleted files"), lShown, lTotal) + L" · "
            + wxString::Format(Tr(L"%d selected (%s)"), iChecked, FormatFileSize(m_pList->GetCheckedBytes())));
    m_pFrame->UpdateFooter();
}

void CFilesPage::OnSearchText(wxCommandEvent &WXUNUSED(event)) {
    m_searchTimer.StartOnce(300);
}

void CFilesPage::OnSearchTimer(wxTimerEvent &WXUNUSED(event)) {
    wxBusyCursor busy; // a few hundred thousand files take a moment
    m_pList->SetNameFilter(m_pSearch->GetValue());
}

void CFilesPage::OnShowFilter(wxCommandEvent &WXUNUSED(event)) {
    wxBusyCursor busy;
    m_pList->SetRecoverabilityFilter(m_pShow->GetSelection());
}

void CFilesPage::OnTypeFilter(wxCommandEvent &WXUNUSED(event)) {
    wxBusyCursor busy;
    m_pList->SetTypeFilter(m_pType->GetSelection());
}

void CFilesPage::OnChecked(wxCommandEvent &WXUNUSED(event)) {
    UpdateSummary();
}

void CFilesPage::OnFocused(wxListEvent &event) {
    m_pDetails->ShowFile(m_pList->GetFile(event.GetIndex()), m_pDrive);
}

// read the start of the ticked files (or of every shown file) and compare it with the file type. Runs in the
// GUI thread like the preview - one small read per file, and the progress dialog keeps the window responsive.

void CFilesPage::OnCheckContents(wxCommandEvent &WXUNUSED(event)) {
    if (!m_pDrive)
        return;
    std::vector<t_FileInfo *> vFiles;
    m_pList->GetCheckedFiles(vFiles);
    if (vFiles.empty())
        for (long lItem = 0; lItem < m_pList->GetFileCount(); lItem++)
            vFiles.push_back(m_pList->GetFile(lItem));
    if (vFiles.empty())
        return;

    wxProgressDialog dialog(Tr(L"Checking contents"), wxString::Format(Tr(L"Checking %d files..."), (int) vFiles.size()),
            (int) vFiles.size(), this, wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME | wxPD_AUTO_HIDE);

    int aCount[CONTENT_UNREADABLE + 1] = {0};
    size_t i;
    DWORD dwLastUpdate = 0;
    for (i = 0; i < vFiles.size(); i++) {
        if (GetTickCount() - dwLastUpdate > 100) {
            dwLastUpdate = GetTickCount();
            if (!dialog.Update((int) i, wxString::Format(Tr(L"Checking file %d of %d"), (int) i + 1, (int) vFiles.size())))
                break; // cancelled - what was checked so far is kept
        }
        if (vFiles[i]->iContent == CONTENT_NOT_CHECKED)
            m_pDrive->CheckFileContents(*vFiles[i]);
        if (vFiles[i]->iContent >= 0 && vFiles[i]->iContent <= CONTENT_UNREADABLE)
            aCount[vFiles[i]->iContent]++;
    }
    dialog.Hide();

    m_pList->Refilter(); // the sort may depend on it
    m_pDetails->ShowFile(m_pDetails->GetFile(), m_pDrive);
    UpdateSummary();

    wxString sSummary = wxString::Format(Tr(L"%d files checked"), (int) i);
    if (i < vFiles.size())
        sSummary += wxString(" ") + Tr(L"(cancelled)");
    sSummary += ":\n\n";
    sSummary += wxString::Format("%s: %d\n", Tr(L"Looks valid"), aCount[CONTENT_OK]);
    sSummary += wxString::Format("%s: %d\n", Tr(L"Damaged"), aCount[CONTENT_DAMAGED] + aCount[CONTENT_ZEROS]);
    sSummary += wxString::Format("%s: %d\n", Tr(L"Start overwritten"), aCount[CONTENT_START_OVERWRITTEN]);
    sSummary += wxString::Format("%s: %d\n", Tr(L"Can't check this file type"), aCount[CONTENT_NO_SIGNATURE]);
    if (aCount[CONTENT_UNREADABLE])
        sSummary += wxString::Format("%s: %d\n", Tr(L"Can't read"), aCount[CONTENT_UNREADABLE]);
    sSummary += wxString("\n") + Tr(L"\"Looks valid\" means the start of the file matches its type - the rest of it can still be damaged.");
    wxMessageBox(sSummary, Tr(L"Contents checked"), wxOK | wxICON_INFORMATION, this);
}

/////////////////////////////////// step 4: done ///////////////////////////////////

CDonePage::CDonePage(wxWindow *parent, CWizardFrame *pFrame)
: wxPanel(parent, wxID_ANY) {
    m_pFrame = pFrame;
    SetBackgroundColour(Theme::PanelBg);

    wxStaticBitmap *pIcon = new wxStaticBitmap(this, wxID_ANY, Theme::Icon(Theme::Icons::CheckCircle, Theme::Accent, 64, 1.6));
    m_pTitle = Label(this, wxEmptyString, Theme::Title(), Theme::Text);
    m_pFolder = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(600, -1)),
            wxST_NO_AUTORESIZE | wxST_ELLIPSIZE_MIDDLE | wxALIGN_CENTER_HORIZONTAL);
    m_pFolder->SetFont(Theme::Body());
    m_pFolder->SetForegroundColour(Theme::Text);
    m_pNote = Label(this, wxEmptyString, Theme::Body(), Theme::TextSecondary);
    m_pFailuresLabel = Label(this, Tr(L"These files could not be recovered:"), Theme::Small(), Theme::Text);
    m_pFailures = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(520, 100)),
            wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);
    m_pFailures->SetFont(Theme::Small());
    CFlatButton *pOpen = new CFlatButton(this, wxID_ANY, Tr(L"Open folder"), FLAT_BUTTON_PRIMARY);
    CFlatButton *pMore = new CFlatButton(this, wxID_ANY, Tr(L"Recover more files"), FLAT_BUTTON_SECONDARY);
    pOpen->Bind(wxEVT_BUTTON, &CDonePage::OnOpenFolder, this);
    pMore->Bind(wxEVT_BUTTON, &CDonePage::OnMore, this);

    wxBoxSizer *pButtons = new wxBoxSizer(wxHORIZONTAL);
    pButtons->Add(pOpen);
    pButtons->Add(pMore, 0, wxLEFT, FromDIP(8));

    wxBoxSizer *pColumn = new wxBoxSizer(wxVERTICAL);
    pColumn->Add(pIcon, 0, wxALIGN_CENTER_HORIZONTAL);
    pColumn->Add(m_pTitle, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, FromDIP(14));
    pColumn->Add(m_pFolder, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, FromDIP(4));
    pColumn->Add(m_pNote, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, FromDIP(12));
    pColumn->Add(m_pFailuresLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, FromDIP(14));
    pColumn->Add(m_pFailures, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, FromDIP(6));
    pColumn->Add(pButtons, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, FromDIP(22));

    wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
    pSizer->AddStretchSpacer();
    pSizer->Add(pColumn, 0, wxEXPAND);
    pSizer->AddStretchSpacer();
    SetSizer(pSizer);
}

void CDonePage::ShowResult(int iRecovered, int iSelected, int iPartly, int iOverwritten, bool bCancelled,
        const std::vector<wxString> &vFailures, const wxString &sDestination) {
    m_sDestination = sDestination;
    wxString sTitle;
    if (bCancelled)
        sTitle = wxString::Format(Tr(L"Recovery stopped - %d of %d files recovered"), iRecovered, iSelected);
    else if (iRecovered == 1)
        sTitle = Tr(L"1 file recovered");
    else
        sTitle = wxString::Format(Tr(L"%d files recovered"), iRecovered);
    m_pTitle->SetLabel(sTitle);
    m_pFolder->SetLabel(wxString::Format(Tr(L"Saved in %s"), sDestination));
    m_pFolder->SetToolTip(sDestination);

    wxString sNote;
    if (iOverwritten)
        sNote = wxString::Format(Tr(L"%d of them were marked as overwritten - other files are stored where their data was, so they will most likely be unreadable."), iOverwritten);
    else if (iPartly)
        sNote = wxString::Format(Tr(L"%d of them were partly overwritten and may not open completely."), iPartly);
    else if (iRecovered)
        sNote = Tr(L"All of them were intact on the drive.");
    m_pNote->SetLabel(sNote);
    m_pNote->Wrap(FromDIP(560));

    wxString sFailures;
    for (size_t i = 0; i < vFailures.size(); i++)
        sFailures += vFailures[i] + "\n";
    m_pFailures->SetValue(sFailures);
    m_pFailuresLabel->Show(!vFailures.empty());
    m_pFailures->Show(!vFailures.empty());
    Layout();
}

void CDonePage::OnOpenFolder(wxCommandEvent &WXUNUSED(event)) {
    ShellExecuteW(NULL, L"explore", m_sDestination.wc_str(), NULL, NULL, SW_SHOWNORMAL);
}

void CDonePage::OnMore(wxCommandEvent &WXUNUSED(event)) {
    m_pFrame->GoToStep(STEP_FILES);
}
