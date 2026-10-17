// MainFrame.cpp: the app's window - drives, the streaming list of deleted files, filters, details, recovery

#include <windows.h>
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/iconbndl.h>
#include <wx/progdlg.h>

#include "MainFrame.h"
#include "RecoverDialog.h"
#include "FileDetailsPanel.h"
#include "FileContents.h"
#include "MyThreadClass.h"
#include "Theme.h"
#include "languages/Translate.h"
#include "version.h"

#ifdef DEMO_BUILD
#define APP_VERSION "DEMO"
#else
#define APP_VERSION "v" APP_VERSION_STR
#endif

enum {
    CENTER_PICK_DRIVE = 0, // nothing scanned yet
    CENTER_WAITING, // scanning, nothing found yet
    CENTER_LIST,
    CENTER_NO_MATCH, // files found, but the filters hide them all
    CENTER_NOTHING_FOUND // the scan found no deleted files
};

static wxStaticText *Label(wxWindow *parent, const wxString &sText, const wxFont &font, const wxColour &colour) {
    wxStaticText *pText = new wxStaticText(parent, wxID_ANY, sText);
    pText->SetFont(font);
    pText->SetForegroundColour(colour);
    return pText;
}

// the white card with a thin rounded outline that holds the filters, the list and the details

class CCardPanel : public wxPanel {
public:
    CCardPanel(wxWindow *parent) : wxPanel(parent, wxID_ANY) {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetBackgroundColour(Theme::PanelBg);
        Bind(wxEVT_PAINT, [this](wxPaintEvent &) {
            wxAutoBufferedPaintDC dcPaint(this);
            dcPaint.SetBackground(wxBrush(GetParent()->GetBackgroundColour()));
            dcPaint.Clear();
            wxGCDC dc(dcPaint);
            Theme::DrawRoundedBox(dc, GetClientRect(), FromDIP(8), Theme::PanelBg, Theme::Border);
        });
    }
};

// a 1 px line between the card's columns

static wxWindow *Separator(wxWindow *parent) {
    wxWindow *pLine = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(1, -1));
    pLine->SetBackgroundColour(Theme::Border);
    pLine->SetMinSize(wxSize(1, -1));
    return pLine;
}

CMainFrame::CMainFrame(const std::vector<wxString> &vImages)
: wxFrame(NULL, wxID_ANY, wxString("GetMyFilesBack ") + APP_VERSION) {
    m_bHasSource = false;
    m_pDrive = NULL;
    m_pScan = NULL;
    m_bScanStopped = false;

    SetIcons(wxIconBundle("#1", GetModuleHandle(NULL))); // the exe's icon (undelete.rc)
    SetBackgroundColour(Theme::WindowBg);
    SetFont(Theme::Body());

    wxPanel *pRoot = new wxPanel(this, wxID_ANY);
    pRoot->SetBackgroundColour(Theme::WindowBg);

    std::vector<ST_DRIVE> vDrives = ListDrives();
    for (size_t i = 0; i < vImages.size(); i++)
        vDrives.push_back(DiskImageDrive(vImages[i]));

    // drives and search
    m_pSearch = new wxSearchCtrl(pRoot, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(260, -1)));
    m_pSearch->SetFont(Theme::Small());
    m_pSearch->SetDescriptiveText(Tr(L"Search name or folder"));
    m_pSearch->ShowCancelButton(true);
    wxBoxSizer *pTop = new wxBoxSizer(wxHORIZONTAL);
    pTop->Add(CreateDriveStrip(pRoot, vDrives), 1, wxALIGN_CENTER_VERTICAL);
    pTop->Add(m_pSearch, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(16));

    // filters | list | details
    CCardPanel *pCard = new CCardPanel(pRoot);
    m_pDetails = new CFileDetailsPanel(pCard, 218);
    m_pDetails->OnContentChecked = [this](t_FileInfo *pFile) { m_pList->RefreshFile(pFile); };
    wxBoxSizer *pCardSizer = new wxBoxSizer(wxHORIZONTAL);
    pCardSizer->Add(CreateFilters(pCard), 0, wxEXPAND | wxALL, FromDIP(16));
    pCardSizer->Add(Separator(pCard), 0, wxEXPAND | wxTOP | wxBOTTOM, 1);
    pCardSizer->Add(CreateCenter(pCard), 1, wxEXPAND | wxTOP | wxBOTTOM, FromDIP(2));
    pCardSizer->Add(Separator(pCard), 0, wxEXPAND | wxTOP | wxBOTTOM, 1);
    pCardSizer->Add(m_pDetails, 0, wxEXPAND | wxALL, FromDIP(16));
    pCard->SetSizer(pCardSizer);

    wxBoxSizer *pRootSizer = new wxBoxSizer(wxVERTICAL);
    pRootSizer->Add(pTop, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, FromDIP(16));
    pRootSizer->Add(pCard, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, FromDIP(12));
    pRootSizer->Add(CreateBottomBar(pRoot), 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(16));
    pRoot->SetSizer(pRootSizer);

    // the list, its 7 columns, the filters and the details - and the whole window still fits a 1366x768 screen
    SetMinClientSize(FromDIP(wxSize(1060, 600)));
    SetClientSize(FromDIP(wxSize(1280, 720)));
    Centre();

    m_scanTimer.SetOwner(this, 1);
    m_searchTimer.SetOwner(this, 2);
    Bind(wxEVT_TIMER, &CMainFrame::OnScanTimer, this, 1);
    Bind(wxEVT_TIMER, &CMainFrame::OnSearchTimer, this, 2);
    Bind(wxEVT_THREAD, &CMainFrame::OnThreadDone, this);
    Bind(wxEVT_CLOSE_WINDOW, &CMainFrame::OnClose, this);
    m_pSearch->Bind(wxEVT_TEXT, &CMainFrame::OnSearchText, this);
    m_pSearch->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, [this](wxCommandEvent &) { m_pSearch->Clear(); });
    m_pList->Bind(EVT_DELETED_FILES_CHECKED, &CMainFrame::OnChecked, this);
    m_pList->Bind(wxEVT_LIST_ITEM_FOCUSED, &CMainFrame::OnFocused, this);

    UpdateCenter();
    UpdateCounts();
    UpdateStatus();
}

CMainFrame::~CMainFrame() {
    DeleteFileInfoHash(&m_hash);
    delete m_pDrive;
}

wxWindow *CMainFrame::CreateDriveStrip(wxWindow *parent, const std::vector<ST_DRIVE> &vDrives) {
    // a row of compact cards - it scrolls sideways if there are more than fit
    wxScrolledWindow *pStrip = new wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL);
    pStrip->SetBackgroundColour(Theme::WindowBg);
    pStrip->SetScrollRate(FromDIP(10), 0);
    wxBoxSizer *pSizer = new wxBoxSizer(wxHORIZONTAL);
    for (size_t i = 0; i < vDrives.size(); i++) {
        CDriveCard *pCard = new CDriveCard(pStrip, wxID_ANY, vDrives[i]);
        pCard->Bind(wxEVT_BUTTON, &CMainFrame::OnDriveCard, this);
        pSizer->Add(pCard, 0, wxRIGHT, FromDIP(10));
        m_vCards.push_back(pCard);
    }
    if (vDrives.empty())
        pSizer->Add(Label(pStrip, Tr(L"No hard disks or USB drives were found."), Theme::Body(), Theme::TextSecondary), 0, wxALIGN_CENTER_VERTICAL);
    pStrip->SetSizer(pSizer);
    pStrip->SetMinSize(wxSize(-1, FromDIP(64) + 2));
    return pStrip;
}

// "TYPE", "RECOVERABLE", "CONTENTS" - the filter groups on the left

static wxStaticText *Legend(wxWindow *parent, const wxString &sText) {
    return Label(parent, sText.Upper(), Theme::Bold(Theme::Tiny()), Theme::TextSecondary);
}

wxWindow *CMainFrame::CreateFilters(wxWindow *parent) {
    wxPanel *pPanel = new wxPanel(parent, wxID_ANY);
    pPanel->SetBackgroundColour(Theme::PanelBg);
    wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);

    pSizer->Add(Legend(pPanel, Tr(L"Type")), 0, wxBOTTOM, FromDIP(6));
    for (int i = FILE_TYPE_ANY; i < FILE_TYPE_COUNT; i++) {
        m_apType[i] = new wxRadioButton(pPanel, wxID_ANY, FileTypeFilterLabel(i), wxDefaultPosition, wxDefaultSize, i == FILE_TYPE_ANY ? wxRB_GROUP : 0);
        m_apType[i]->Bind(wxEVT_RADIOBUTTON, &CMainFrame::OnTypeFilter, this);
        m_apTypeCount[i] = Label(pPanel, wxEmptyString, Theme::Tiny(), Theme::TextSecondary);
        wxBoxSizer *pRow = new wxBoxSizer(wxHORIZONTAL);
        pRow->Add(m_apType[i], 1, wxALIGN_CENTER_VERTICAL);
        pRow->Add(m_apTypeCount[i], 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(8));
        pRow->SetMinSize(-1, FromDIP(26));
        pSizer->Add(pRow, 0, wxEXPAND);
    }

    pSizer->AddSpacer(FromDIP(12));
    pSizer->Add(Legend(pPanel, Tr(L"Recoverable")), 0, wxBOTTOM, FromDIP(6));
    for (int i = RECOVERABILITY_FILTER_ALL; i < RECOVERABILITY_FILTER_COUNT; i++) {
        m_apShow[i] = new wxRadioButton(pPanel, wxID_ANY, RecoverabilityFilterLabel(i), wxDefaultPosition, wxDefaultSize, i == RECOVERABILITY_FILTER_ALL ? wxRB_GROUP : 0);
        m_apShow[i]->Bind(wxEVT_RADIOBUTTON, &CMainFrame::OnShowFilter, this);
        m_apShow[i]->SetMinSize(wxSize(-1, FromDIP(26)));
        pSizer->Add(m_apShow[i], 0, wxEXPAND);
    }

    pSizer->AddSpacer(FromDIP(12));
    pSizer->Add(Legend(pPanel, Tr(L"Contents")), 0, wxBOTTOM, FromDIP(6));
    m_pValidOnly = new wxCheckBox(pPanel, wxID_ANY, Tr(L"Looks valid only"));
    m_pValidOnly->SetMinSize(wxSize(-1, FromDIP(26)));
    m_pValidOnly->Bind(wxEVT_CHECKBOX, &CMainFrame::OnValidOnly, this);
    pSizer->Add(m_pValidOnly, 0, wxEXPAND);
    m_pCheckContents = new CFlatButton(pPanel, wxID_ANY, Tr(L"Check contents"), FLAT_BUTTON_SECONDARY);
    m_pCheckContents->Bind(wxEVT_BUTTON, &CMainFrame::OnCheckContents, this);
    pSizer->Add(m_pCheckContents, 0, wxTOP, FromDIP(8));
    wxStaticText *pNote = Label(pPanel, Tr(L"Reads the start of each shown file and compares it with its type."), Theme::Tiny(), Theme::TextSecondary);
    pNote->Wrap(FromDIP(178));
    pSizer->Add(pNote, 0, wxTOP, FromDIP(8));

    for (int i = 0; i < FILE_TYPE_COUNT; i++)
        m_apType[i]->SetFont(Theme::Body());
    for (int i = 0; i < RECOVERABILITY_FILTER_COUNT; i++)
        m_apShow[i]->SetFont(Theme::Body());
    m_pValidOnly->SetFont(Theme::Body());

    pSizer->SetMinSize(FromDIP(wxSize(178, -1)));
    pPanel->SetSizer(pSizer);
    return pPanel;
}

// a big grey icon and a message, in the middle of the list's place

static wxPanel *Placeholder(wxWindow *parent, const char *szIcon, const wxString &sTitle, const wxString &sText, wxWindow **ppExtra = NULL) {
    wxPanel *pPanel = new wxPanel(parent, wxID_ANY);
    pPanel->SetBackgroundColour(Theme::PanelBg);
    wxBoxSizer *pColumn = new wxBoxSizer(wxVERTICAL);
    if (szIcon)
        pColumn->Add(new wxStaticBitmap(pPanel, wxID_ANY, Theme::Icon(szIcon, Theme::TextDisabled, 56, 1.2)), 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, pPanel->FromDIP(10));
    if (!sTitle.IsEmpty()) {
        wxStaticText *pTitle = Label(pPanel, sTitle, Theme::Bold(Theme::Pixels(18)), Theme::Text);
        pColumn->Add(pTitle, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, pPanel->FromDIP(10));
    }
    wxStaticText *pText = Label(pPanel, sText, Theme::Body(), Theme::TextSecondary);
    pText->Wrap(pPanel->FromDIP(420));
    pColumn->Add(pText, 0, wxALIGN_CENTER_HORIZONTAL);
    if (ppExtra) {
        CFlatButton *pButton = new CFlatButton(pPanel, wxID_ANY, Tr(L"Clear filters"), FLAT_BUTTON_SECONDARY);
        pColumn->Add(pButton, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, pPanel->FromDIP(12));
        *ppExtra = pButton;
    }
    wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
    pSizer->AddStretchSpacer();
    pSizer->Add(pColumn, 0, wxEXPAND | wxLEFT | wxRIGHT, pPanel->FromDIP(24));
    pSizer->AddStretchSpacer();
    pPanel->SetSizer(pSizer);
    return pPanel;
}

wxWindow *CMainFrame::CreateCenter(wxWindow *parent) {
    m_pCenter = new wxSimplebook(parent, wxID_ANY);
    m_pCenter->SetBackgroundColour(Theme::PanelBg);
    m_pCenter->AddPage(Placeholder(m_pCenter, Theme::Icons::HardDisk, Tr(L"Pick a drive to look for deleted files"),
            Tr(L"Choose the drive above that held the files. Deleted files show up here while the drive is being scanned - you can start ticking them right away.")), wxEmptyString);
    m_pCenter->AddPage(Placeholder(m_pCenter, NULL, wxEmptyString, Tr(L"Scanning - the first deleted files appear in a moment...")), wxEmptyString);
    m_pList = new CDeletedFilesList(m_pCenter, wxID_ANY);
    m_pCenter->AddPage(m_pList, wxEmptyString);
    wxWindow *pClear = NULL;
    m_pCenter->AddPage(Placeholder(m_pCenter, NULL, wxEmptyString, Tr(L"No files match these filters."), &pClear), wxEmptyString);
    pClear->Bind(wxEVT_BUTTON, &CMainFrame::OnClearFilters, this);
    m_pCenter->AddPage(Placeholder(m_pCenter, NULL, Tr(L"No deleted files found"),
            Tr(L"There are no deleted files on this drive that could be recovered.")), wxEmptyString);
    return m_pCenter;
}

wxWindow *CMainFrame::CreateBottomBar(wxWindow *parent) {
    wxPanel *pBar = new wxPanel(parent, wxID_ANY);
    pBar->SetBackgroundColour(Theme::WindowBg);
    m_pBar = new CThinBar(pBar);
    m_pBar->SetMinSize(FromDIP(wxSize(180, 6)));
    m_pStatus = Label(pBar, wxEmptyString, Theme::Small(), wxColour(0x3B, 0x3B, 0x3B));
    m_pStop = new CFlatButton(pBar, wxID_ANY, Tr(L"Stop scan"), FLAT_BUTTON_SECONDARY);
    m_pRecover = new CFlatButton(pBar, wxID_ANY, Tr(L"Recover..."), FLAT_BUTTON_PRIMARY);
    m_pStop->Bind(wxEVT_BUTTON, &CMainFrame::OnStopScan, this);
    m_pRecover->Bind(wxEVT_BUTTON, &CMainFrame::OnRecover, this);

    wxBoxSizer *pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_pBar, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(14));
    pSizer->Add(m_pStatus, 1, wxALIGN_CENTER_VERTICAL);
    pSizer->Add(m_pStop, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(14));
    pSizer->Add(m_pRecover, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(14));
    pSizer->SetMinSize(-1, FromDIP(64));
    pBar->SetSizer(pSizer);
    return pBar;
}

/////////////////////////////////// the scan ///////////////////////////////////

void CMainFrame::OnDriveCard(wxCommandEvent &event) {
    CDriveCard *pCard = (CDriveCard *) event.GetEventObject();
    if (pCard->GetDrive().bSupported)
        StartScan(pCard->GetDrive());
}

void CMainFrame::StartScan(const ST_DRIVE &drive) {
    if (m_pScan) { // another drive picked while one is being scanned - that scan goes
        m_pScan->Cancel();
        StopScan();
    }

    // the list and the details point into the hash the scan is about to refill, and the old engine goes too
    m_pDetails->ShowFile(NULL, NULL);
    m_pList->Clear();
    delete m_pDrive;
    m_pDrive = NULL;
    m_source = drive;
    m_bHasSource = true;
    m_bScanStopped = false;
    for (size_t i = 0; i < m_vCards.size(); i++)
        m_vCards[i]->SetSelected(m_vCards[i]->GetDrive().sSource == drive.sSource);

    m_pScan = new CScanThread(this, drive.sSource, &m_hash);
    if (m_pScan->Run() != wxTHREAD_NO_ERROR) {
        delete m_pScan;
        m_pScan = NULL;
        wxMessageBox(Tr(L"Can't create thread for reading the drive."), Tr(L"Error"), wxOK | wxICON_ERROR, this);
        return;
    }
    m_scanTimer.Start(250);
    UpdateCenter();
    UpdateCounts();
    UpdateStatus();
}

void CMainFrame::TakeNewFiles() {
    std::vector<t_FileInfo *> vNew;
    m_pScan->TakeNewFiles(vNew);
    if (vNew.empty())
        return;
    m_pList->AddFiles(vNew);
    UpdateCounts();
    if (m_pCenter->GetSelection() == CENTER_WAITING)
        UpdateCenter();
}

void CMainFrame::OnScanTimer(wxTimerEvent &WXUNUSED(event)) {
    if (!m_pScan)
        return;
    TakeNewFiles();
    UpdateStatus();
}

void CMainFrame::OnThreadDone(wxThreadEvent &event) {
    if (event.GetId() == THREAD_SCAN_DONE && m_pScan)
        StopScan();
}

void CMainFrame::OnStopScan(wxCommandEvent &WXUNUSED(event)) {
    if (m_pScan) {
        m_pScan->Cancel(); // the list keeps what was found so far
        StopScan();
    }
}

// The scan thread is done (or told to stop): wait for it, then complete its files in this thread - the
// engine's FinishScan changes them (folders, Recycle Bin names) and deletes the Recycle Bin's $I files, so
// nothing may be reading them meanwhile. The list then takes the whole result over, sorted, ticks kept.

void CMainFrame::StopScan() {
    {
        wxBusyCursor busy;
        m_pScan->Wait();
    }
    m_scanTimer.Stop();
    TakeNewFiles();
    int iResult = m_pScan->m_iResult;
    m_pDrive = m_pScan->m_pDrive;
    delete m_pScan;
    m_pScan = NULL;
    wxLog::FlushActive(); // the thread's log lines

    if (iResult && iResult != ERROR_CANCELLED) {
        m_pList->Clear();
        DeleteFileInfoHash(&m_hash);
        delete m_pDrive;
        m_pDrive = NULL;
        m_bHasSource = false;
        for (size_t i = 0; i < m_vCards.size(); i++)
            m_vCards[i]->SetSelected(false);
        UpdateCenter();
        UpdateCounts();
        UpdateStatus();

        wxString sReason;
        if (iResult == ERROR_ACCESS_DENIED)
            sReason = Tr(L"Access to the drive was denied. Please run the program as administrator.");
        else if (iResult == ERROR_INVALID_DRIVE || iResult == ERROR_BAD_DEVICE)
            sReason = Tr(L"This drive holds neither an NTFS nor a FAT file system - files can't be recovered from it.");
        else
            sReason.Printf(Tr(L"The drive could not be read (error %d)."), iResult);
        wxMessageBox(sReason, Tr(L"Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    m_bScanStopped = iResult == ERROR_CANCELLED;
    m_pDetails->ShowFile(NULL, NULL); // it may show a Recycle Bin $I file, which FinishScan deletes
    if (m_pDrive)
        m_pDrive->FinishScan(&m_hash);
    m_pList->SetFiles(&m_hash);
    m_pDetails->ShowFile(m_pList->GetFocusedFile(), m_pDrive);
    UpdateCenter();
    UpdateCounts();
    UpdateStatus();
}

/////////////////////////////////// what is shown ///////////////////////////////////

void CMainFrame::UpdateCenter() {
    int iPage = CENTER_LIST;
    if (!m_bHasSource)
        iPage = CENTER_PICK_DRIVE;
    else if (m_pList->GetTotalFileCount() == 0)
        iPage = m_pScan ? CENTER_WAITING : CENTER_NOTHING_FOUND;
    else if (m_pList->GetFileCount() == 0)
        iPage = CENTER_NO_MATCH;
    if (m_pCenter->GetSelection() != iPage)
        m_pCenter->ChangeSelection(iPage);

    // nothing else may read the drive while the scan does
    bool bCanRead = DriveForReading() != NULL;
    m_pCheckContents->Enable(bCanRead && m_pList->GetFileCount() > 0);
    m_pValidOnly->Enable(bCanRead || m_pValidOnly->GetValue());
}

void CMainFrame::UpdateCounts() {
    int aCounts[FILE_TYPE_COUNT];
    m_pList->CountByType(aCounts);
    for (int i = 0; i < FILE_TYPE_COUNT; i++)
        m_apTypeCount[i]->SetLabel(m_bHasSource ? wxString::Format("%d", aCounts[i]) : wxString());
    m_apTypeCount[0]->GetParent()->Layout();
}

void CMainFrame::UpdateStatus() {
    int iChecked = m_pList->GetCheckedCount();
    long lFound = m_pList->GetTotalFileCount(), lShown = m_pList->GetFileCount();
    wxString sStatus;
    if (!m_bHasSource) {
        sStatus = Tr(L"No drive scanned yet.");
    } else if (m_pScan) {
        int iPercent = m_pScan->GetPercent();
        sStatus = wxString::Format(Tr(L"Scanning %s"), m_source.sName);
        if (iPercent >= 0)
            sStatus += wxString::Format(L" · %d %%", iPercent);
        sStatus += L" · " + wxString::Format(Tr(L"%ld deleted files found so far - you can tick files already."), lFound);
    } else {
        sStatus = wxString::Format(m_bScanStopped ? Tr(L"Scan of %s stopped") : Tr(L"Scan of %s finished"), m_source.sName)
                + L" · " + wxString::Format(Tr(L"%ld deleted files"), lFound)
                + L" · " + wxString::Format(Tr(L"showing %ld"), lShown);
        if (iChecked)
            sStatus += L" · " + wxString::Format(Tr(L"%d selected"), iChecked);
    }
    m_pStatus->SetLabel(sStatus);

    m_pBar->Show(m_bHasSource);
    if (m_pScan) {
        m_pBar->SetColour(Theme::Accent);
        m_pBar->SetValue(m_pScan->GetPercent());
    } else {
        m_pBar->SetColour(m_bScanStopped ? Theme::PartialDot : Theme::GoodDot);
        m_pBar->SetValue(100);
    }
    m_pStop->Show(m_pScan != NULL);

    wxString sRecover = Tr(L"Recover...");
    if (iChecked == 1)
        sRecover = wxString::Format(Tr(L"Recover 1 file (%s)..."), FormatFileSize(m_pList->GetCheckedBytes()));
    else if (iChecked > 1)
        sRecover = wxString::Format(Tr(L"Recover %d files (%s)..."), iChecked, FormatFileSize(m_pList->GetCheckedBytes()));
    m_pRecover->SetLabel(sRecover);
    m_pRecover->Enable(iChecked > 0);
    m_pStatus->GetParent()->Layout();
}

void CMainFrame::FiltersChanged() {
    UpdateCenter();
    UpdateCounts();
    UpdateStatus();
}

/////////////////////////////////// filters ///////////////////////////////////

void CMainFrame::OnSearchText(wxCommandEvent &WXUNUSED(event)) {
    m_searchTimer.StartOnce(300);
}

void CMainFrame::OnSearchTimer(wxTimerEvent &WXUNUSED(event)) {
    wxBusyCursor busy; // a few hundred thousand files take a moment
    m_pList->SetNameFilter(m_pSearch->GetValue());
    FiltersChanged();
}

void CMainFrame::OnTypeFilter(wxCommandEvent &WXUNUSED(event)) {
    wxBusyCursor busy;
    for (int i = 0; i < FILE_TYPE_COUNT; i++)
        if (m_apType[i]->GetValue())
            m_pList->SetTypeFilter(i);
    FiltersChanged();
}

void CMainFrame::OnShowFilter(wxCommandEvent &WXUNUSED(event)) {
    wxBusyCursor busy;
    for (int i = 0; i < RECOVERABILITY_FILTER_COUNT; i++)
        if (m_apShow[i]->GetValue())
            m_pList->SetRecoverabilityFilter(i);
    FiltersChanged();
}

// "Looks valid only" needs the contents checked - the shown files that aren't yet are checked first

void CMainFrame::OnValidOnly(wxCommandEvent &WXUNUSED(event)) {
    bool bOn = m_pValidOnly->GetValue();
    if (bOn && DriveForReading()) {
        std::vector<t_FileInfo *> vToCheck;
        for (long i = 0; i < m_pList->GetFileCount(); i++)
            if (m_pList->GetFile(i)->iContent == CONTENT_NOT_CHECKED)
                vToCheck.push_back(m_pList->GetFile(i));
        if (!vToCheck.empty() && !CheckContents(vToCheck, false)) {
            m_pValidOnly->SetValue(false); // cancelled - not every shown file could be judged
            return;
        }
    }
    wxBusyCursor busy;
    m_pList->SetValidOnly(bOn);
    FiltersChanged();
}

void CMainFrame::OnCheckContents(wxCommandEvent &WXUNUSED(event)) {
    std::vector<t_FileInfo *> vFiles;
    for (long i = 0; i < m_pList->GetFileCount(); i++)
        vFiles.push_back(m_pList->GetFile(i));
    CheckContents(vFiles, true);
}

void CMainFrame::OnClearFilters(wxCommandEvent &WXUNUSED(event)) {
    m_pSearch->ChangeValue(wxEmptyString);
    m_apType[FILE_TYPE_ANY]->SetValue(true);
    m_apShow[RECOVERABILITY_FILTER_ALL]->SetValue(true);
    m_pValidOnly->SetValue(false);
    m_pList->ClearFilters();
    FiltersChanged();
}

// Read the start of the files and compare it with their type - in this thread, like the preview: one small
// read per file, and the progress dialog keeps the window responsive. False if it was cancelled.

bool CMainFrame::CheckContents(const std::vector<t_FileInfo *> &vFiles, bool bShowSummary) {
    CRecoveryDrive *pDrive = DriveForReading();
    if (!pDrive || vFiles.empty())
        return true;

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
            pDrive->CheckFileContents(*vFiles[i]);
        if (vFiles[i]->iContent >= 0 && vFiles[i]->iContent <= CONTENT_UNREADABLE)
            aCount[vFiles[i]->iContent]++;
    }
    dialog.Hide();
    m_pList->Refilter(); // the sort may depend on it
    m_pDetails->ShowFile(m_pDetails->GetFile(), pDrive);
    FiltersChanged();

    if (bShowSummary) {
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
    return i == vFiles.size();
}

/////////////////////////////////// the list ///////////////////////////////////

void CMainFrame::OnChecked(wxCommandEvent &WXUNUSED(event)) {
    UpdateStatus();
}

void CMainFrame::OnFocused(wxListEvent &event) {
    m_pDetails->ShowFile(m_pList->GetFile(event.GetIndex()), DriveForReading());
}

/////////////////////////////////// recovery ///////////////////////////////////

void CMainFrame::OnRecover(wxCommandEvent &WXUNUSED(event)) {
    if (m_pScan) { // the copying needs the drive to itself - the scan stops, what it found stays
        m_pScan->Cancel();
        StopScan();
    }
    std::vector<t_FileInfo *> vFiles;
    m_pList->GetCheckedFiles(vFiles);
    if (vFiles.empty() || !m_pDrive)
        return;

#ifdef DEMO_BUILD
    wxMessageBox(wxT("This is a DEMO version!\nFile saving is not possible.\nTo buy full version, visit www.getmyfilesback.com"),
            Tr(L"Error"), wxOK | wxICON_ERROR, this);
    return;
#endif

    m_pDetails->ShowFile(m_pDetails->GetFile(), NULL); // the dialog's thread reads the drive now
    CRecoverDialog dialog(this, m_pDrive, m_source, vFiles, m_sLastDestination);
    dialog.ShowModal();
    if (!dialog.GetDestination().IsEmpty())
        m_sLastDestination = dialog.GetDestination();
    m_pDetails->ShowFile(m_pDetails->GetFile(), m_pDrive);
}

void CMainFrame::OnClose(wxCloseEvent &event) {
    // a scan still reading the drive stops first - it uses the engine and the hash this window owns
    if (m_pScan) {
        wxBusyCursor busy;
        m_pScan->Cancel();
        m_pScan->Wait();
        delete m_pScan->m_pDrive;
        delete m_pScan;
        m_pScan = NULL;
    }
    m_scanTimer.Stop();
    event.Skip();
}
