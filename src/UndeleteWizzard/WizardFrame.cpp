// WizardFrame.cpp: the app's window - the steps, the footer, and the scan and recovery threads

#include <windows.h>
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/filename.h>
#include <wx/iconbndl.h>

#include "WizardFrame.h"
#include "DeletedFilesList.h"
#include "FileDetailsPanel.h"
#include "MyThreadClass.h"
#include "AppLog.h"
#include "Theme.h"
#include "languages/Translate.h"

#ifdef DEMO_BUILD
#define APP_VERSION "DEMO"
#else
#define APP_VERSION "v2.06"
#endif

// the white area of the current step, rounded at the top, open at the bottom where the footer is

class CContentPanel : public wxPanel {
public:
    CContentPanel(wxWindow *parent) : wxPanel(parent, wxID_ANY) {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetBackgroundColour(Theme::PanelBg);
        Bind(wxEVT_PAINT, [this](wxPaintEvent &) {
            wxAutoBufferedPaintDC dcPaint(this);
            dcPaint.SetBackground(wxBrush(Theme::WindowBg));
            dcPaint.Clear();
            wxGCDC dc(dcPaint);
            wxRect rect = GetClientRect();
            rect.height += FromDIP(10); // the bottom corners go under the footer
            Theme::DrawRoundedBox(dc, rect, FromDIP(8), Theme::PanelBg, Theme::Border);
        });
    }
};

// the bar at the bottom with the buttons - a line along its top

class CFooterPanel : public wxPanel {
public:
    CFooterPanel(wxWindow *parent) : wxPanel(parent, wxID_ANY) {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetBackgroundColour(Theme::FooterBg);
        Bind(wxEVT_PAINT, [this](wxPaintEvent &) {
            wxAutoBufferedPaintDC dc(this);
            dc.SetBackground(wxBrush(Theme::FooterBg));
            dc.Clear();
            dc.SetPen(wxPen(Theme::Border));
            dc.DrawLine(0, 0, GetClientSize().x, 0);
        });
    }
};

CWizardFrame::CWizardFrame(const std::vector<wxString> &vImages)
: wxFrame(NULL, wxID_ANY, wxString("GetMyFilesBack ") + APP_VERSION) {
    m_iStep = STEP_SOURCE;
    m_bHasSource = false;
    m_bScanStopped = false;
    m_pDrive = NULL;
    m_pScan = NULL;
    m_pCopy = NULL;
    m_llCopyBytes = 0;

    SetIcons(wxIconBundle("#1", GetModuleHandle(NULL))); // the exe's icon (undelete.rc)
    SetBackgroundColour(Theme::WindowBg);
    SetFont(Theme::Body());

    wxPanel *pRoot = new wxPanel(this, wxID_ANY);
    pRoot->SetBackgroundColour(Theme::WindowBg);

    std::vector<wxString> vSteps;
    vSteps.push_back(Tr(L"Source drive"));
    vSteps.push_back(Tr(L"Destination"));
    vSteps.push_back(Tr(L"Select files"));
    vSteps.push_back(Tr(L"Done"));
    m_pSteps = new CStepList(pRoot, vSteps);

    // the current step: heading, a line under it, the page
    CContentPanel *pContent = new CContentPanel(pRoot);
    m_pHeading = new wxStaticText(pContent, wxID_ANY, wxEmptyString);
    m_pHeading->SetFont(Theme::Heading());
    m_pHeading->SetForegroundColour(Theme::Text);
    m_pSubheading = new wxStaticText(pContent, wxID_ANY, wxEmptyString);
    m_pSubheading->SetFont(Theme::Body());
    m_pSubheading->SetForegroundColour(Theme::TextSecondary);
    m_pBook = new wxSimplebook(pContent, wxID_ANY);
    m_pBook->SetBackgroundColour(Theme::PanelBg);

    std::vector<ST_DRIVE> vDrives = ListDrives();
    for (size_t i = 0; i < vImages.size(); i++)
        vDrives.push_back(DiskImageDrive(vImages[i]));
    m_pSourcePage = new CSourcePage(m_pBook, this, vDrives);
    m_pDestinationPage = new CDestinationPage(m_pBook, this);
    m_pFilesPage = new CFilesPage(m_pBook, this);
    m_pDonePage = new CDonePage(m_pBook, this);
    m_pBook->AddPage(m_pSourcePage, wxEmptyString);
    m_pBook->AddPage(m_pDestinationPage, wxEmptyString);
    m_pBook->AddPage(m_pFilesPage, wxEmptyString);
    m_pBook->AddPage(m_pDonePage, wxEmptyString);

    wxBoxSizer *pColumn = new wxBoxSizer(wxVERTICAL);
    pColumn->Add(m_pHeading, 0);
    pColumn->Add(m_pSubheading, 0, wxTOP, FromDIP(4));
    pColumn->Add(m_pBook, 1, wxEXPAND | wxTOP, FromDIP(16));
    wxBoxSizer *pContentSizer = new wxBoxSizer(wxVERTICAL);
    pContentSizer->Add(pColumn, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, FromDIP(24));
    pContentSizer->AddSpacer(FromDIP(16));
    pContent->SetSizer(pContentSizer);
    pContent->Bind(wxEVT_SIZE, [this](wxSizeEvent &event) {
        event.Skip();
        CallAfter([this] { WrapSubheading(); });
    });

    // the footer: a hint, Back, Next, Cancel
    CFooterPanel *pFooter = new CFooterPanel(pRoot);
    m_pHint = new wxStaticText(pFooter, wxID_ANY, wxEmptyString);
    m_pHint->SetFont(Theme::Small());
    m_pHint->SetForegroundColour(Theme::TextSecondary);
    m_pBack = new CFlatButton(pFooter, wxID_BACKWARD, Tr(L"Back"), FLAT_BUTTON_SECONDARY);
    m_pNext = new CFlatButton(pFooter, wxID_FORWARD, Tr(L"Next"), FLAT_BUTTON_PRIMARY);
    m_pCancel = new CFlatButton(pFooter, wxID_CANCEL, Tr(L"Cancel"), FLAT_BUTTON_SECONDARY);
    wxBoxSizer *pFooterSizer = new wxBoxSizer(wxHORIZONTAL);
    pFooterSizer->Add(m_pHint, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(12));
    pFooterSizer->Add(m_pBack, 0, wxALIGN_CENTER_VERTICAL);
    pFooterSizer->Add(m_pNext, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(8));
    pFooterSizer->Add(m_pCancel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(8));
    wxBoxSizer *pFooterOuter = new wxBoxSizer(wxVERTICAL);
    pFooterOuter->Add(pFooterSizer, 1, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(24));
    pFooterOuter->SetMinSize(-1, FromDIP(64));
    pFooter->SetSizer(pFooterOuter);

    wxBoxSizer *pMiddle = new wxBoxSizer(wxHORIZONTAL);
    pMiddle->Add(m_pSteps, 0, wxEXPAND);
    pMiddle->Add(pContent, 1, wxEXPAND | wxTOP | wxRIGHT, FromDIP(16));
    wxBoxSizer *pRootSizer = new wxBoxSizer(wxVERTICAL);
    pRootSizer->Add(pMiddle, 1, wxEXPAND);
    pRootSizer->Add(pFooter, 0, wxEXPAND);
    pRoot->SetSizer(pRootSizer);

    // the list, its 7 columns and the details pane - and the whole window still fits a 1366x768 screen
    SetMinClientSize(FromDIP(wxSize(1060, 620)));
    SetClientSize(FromDIP(wxSize(1280, 720)));
    Centre();

    m_progressTimer.SetOwner(this);
    Bind(wxEVT_TIMER, &CWizardFrame::OnProgressTimer, this);
    Bind(wxEVT_THREAD, &CWizardFrame::OnThreadDone, this);
    Bind(wxEVT_CLOSE_WINDOW, &CWizardFrame::OnClose, this);
    m_pBack->Bind(wxEVT_BUTTON, &CWizardFrame::OnBack, this);
    m_pNext->Bind(wxEVT_BUTTON, &CWizardFrame::OnNext, this);
    m_pCancel->Bind(wxEVT_BUTTON, &CWizardFrame::OnCancel, this);

    GoToStep(STEP_SOURCE);
}

CWizardFrame::~CWizardFrame() {
    DeleteFileInfoHash(&m_hash);
    delete m_pDrive;
}

void CWizardFrame::SetSource(const ST_DRIVE &drive) {
    m_source = drive;
    m_bHasSource = true;
    m_pSourcePage->ShowSelection(drive.sSource);
    UpdateFooter();
}

void CWizardFrame::SetHeading(const wxString &sHeading, const wxString &sSubheading) {
    m_pHeading->SetLabel(sHeading);
    m_sSubheading = sSubheading;
    m_pSubheading->Show(!sSubheading.IsEmpty());
    WrapSubheading();
}

// a line too long for the window wraps - a Windows label would wrap it on its own, but out of sight

void CWizardFrame::WrapSubheading() {
    wxWindow *pContent = m_pHeading->GetParent();
    int iWidth = pContent->GetClientSize().x - FromDIP(48);
    m_pSubheading->SetLabel(m_sSubheading);
    if (iWidth > 100)
        m_pSubheading->Wrap(iWidth);
    pContent->Layout();
}

void CWizardFrame::GoToStep(int iStep) {
    m_iStep = iStep;
    m_pBook->ChangeSelection(iStep);
    m_pSteps->SetCurrent(iStep);

    switch (iStep) {
        case STEP_SOURCE:
            SetHeading(Tr(L"Where were the files deleted from?"),
                    Tr(L"Pick the drive that held the deleted files. The sooner you scan it, the more of them can be recovered."));
            break;
        case STEP_DESTINATION:
            SetHeading(Tr(L"Where should recovered files go?"),
                    Tr(L"Choose a folder on a different drive, so recovering cannot overwrite deleted data."));
            m_pDestinationPage->OnShow();
            break;
        case STEP_FILES:
            SetHeading(Tr(L"Select the files to recover"), m_bScanStopped
                    ? Tr(L"The scan was stopped - only part of the drive was searched.")
                    : Tr(L"Tick the files you want back. Green ones are most likely intact."));
            m_pFilesPage->ShowListAgain();
            break;
        case STEP_DONE:
            SetHeading(Tr(L"Recovery finished"), wxEmptyString);
            break;
    }
    UpdateFooter();
}

void CWizardFrame::UpdateFooter() {
    wxString sHint, sNext = Tr(L"Next"), sCancel = Tr(L"Cancel");
    bool bBack = true, bNext = true, bCancel = true;
    wxString sDrive = m_bHasSource ? m_source.sName : wxString();

    switch (m_iStep) {
        case STEP_SOURCE:
            sHint = m_bHasSource ? wxString::Format(Tr(L"Tip: don't save anything to %s until the recovery is done."), sDrive)
                    : wxString(Tr(L"Pick the drive that held the deleted files."));
            bBack = false;
            bNext = m_bHasSource;
            break;
        case STEP_DESTINATION:
            sHint = wxString::Format(Tr(L"Recovering from %s."), sDrive);
            sNext = Tr(L"Scan drive");
            bNext = m_pDestinationPage->IsValid();
            break;
        case STEP_FILES:
            if (m_pScan || m_pCopy) {
                bBack = bNext = false;
                sNext = Tr(L"Recover");
                sCancel = m_pScan ? Tr(L"Stop scan") : Tr(L"Stop");
            } else {
                int iChecked = m_pFilesPage->GetList()->GetCheckedCount();
                sNext = iChecked == 1 ? wxString(Tr(L"Recover 1 file")) : (iChecked ? wxString::Format(Tr(L"Recover %d files"), iChecked) : wxString(Tr(L"Recover")));
                bNext = iChecked > 0;
                sHint = iChecked ? wxString::Format(Tr(L"%d selected"), iChecked) + L" · " + FormatFileSize(m_pFilesPage->GetList()->GetCheckedBytes())
                        : wxString(Tr(L"Nothing selected yet."));
            }
            break;
        case STEP_DONE:
            sNext = Tr(L"Close");
            bCancel = false;
            break;
    }
    m_pHint->SetLabel(sHint);
    m_pBack->Enable(bBack);
    m_pNext->SetLabel(sNext);
    m_pNext->Enable(bNext);
    m_pCancel->SetLabel(sCancel);
    m_pCancel->Show(bCancel);
    m_pCancel->GetParent()->Layout();
}

void CWizardFrame::OnBack(wxCommandEvent &WXUNUSED(event)) {
    if (IsBusy())
        return;
    if (m_iStep == STEP_DONE)
        GoToStep(STEP_FILES);
    else if (m_iStep > STEP_SOURCE)
        GoToStep(m_iStep - 1);
}

void CWizardFrame::OnNext(wxCommandEvent &WXUNUSED(event)) {
    if (IsBusy())
        return;
    switch (m_iStep) {
        case STEP_SOURCE:
            if (m_bHasSource)
                GoToStep(STEP_DESTINATION);
            break;
        case STEP_DESTINATION: {
            if (!m_pDestinationPage->IsValid())
                return;
            wxString sDestination = m_pDestinationPage->GetDestination();
            if (!wxFileName::DirExists(sDestination) && !wxFileName::Mkdir(sDestination, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
                wxMessageBox(wxString::Format(Tr(L"The folder %s can't be created."), sDestination), Tr(L"Error"), wxOK | wxICON_ERROR, this);
                return;
            }
            m_sDestination = sDestination;
            // from now on the log goes to a file in the destination - what was logged so far goes there too
            CAppLog::Get()->OpenFile(sDestination);
            wxLogMessage("Source: %s (%s), destination: %s", m_source.sSource, m_source.sFileSystem, sDestination);
            StartScan();
            break;
        }
        case STEP_FILES:
            StartRecovery();
            break;
        case STEP_DONE:
            Close();
            break;
    }
}

void CWizardFrame::OnCancel(wxCommandEvent &WXUNUSED(event)) {
    if (m_pScan)
        m_pScan->Cancel(); // the list shows what was found so far
    else if (m_pCopy)
        m_pCopy->Cancel(); // the files recovered so far stay
    else
        Close();
}

/////////////////////////////////// the scan ///////////////////////////////////

void CWizardFrame::StartScan() {
    // the list points into the hash the scan is about to refill, and the old engine goes with it
    m_pFilesPage->ClearList();
    m_pFilesPage->SetDriveForPreview(NULL);
    delete m_pDrive;
    m_pDrive = NULL;
    m_bScanStopped = false;

    m_pScan = new CScanThread(this, m_source.sSource, &m_hash);
    if (m_pScan->Run() != wxTHREAD_NO_ERROR) {
        delete m_pScan;
        m_pScan = NULL;
        wxMessageBox(Tr(L"Can't create thread for reading the drive."), Tr(L"Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    m_iStep = STEP_FILES;
    m_pBook->ChangeSelection(STEP_FILES);
    m_pSteps->SetCurrent(STEP_FILES);
    SetHeading(Tr(L"Looking for deleted files"), Tr(L"This takes from a few seconds to a few minutes, depending on the drive."));
    m_pFilesPage->ShowProgress(wxString::Format(Tr(L"Scanning %s..."), m_source.sName),
            Tr(L"Reading the drive's file table. Nothing is written to the drive."));
    m_progressTimer.Start(150);
    UpdateFooter();
}

void CWizardFrame::OnScanDone() {
    m_pScan->Wait();
    int iResult = m_pScan->m_iResult;
    m_pDrive = m_pScan->m_pDrive;
    delete m_pScan;
    m_pScan = NULL;
    m_progressTimer.Stop();
    wxLog::FlushActive(); // the thread's log lines

    if (iResult && iResult != ERROR_CANCELLED) {
        wxString sReason;
        if (iResult == ERROR_ACCESS_DENIED)
            sReason = Tr(L"Access to the drive was denied. Please run the program as administrator.");
        else if (iResult == ERROR_INVALID_DRIVE || iResult == ERROR_BAD_DEVICE)
            sReason = Tr(L"This drive holds neither an NTFS nor a FAT file system - files can't be recovered from it.");
        else
            sReason.Printf(Tr(L"The drive could not be read (error %d)."), iResult);
        wxMessageBox(sReason + "\n\n" + Tr(L"Details are in the log file in the destination folder."), Tr(L"Error"),
                wxOK | wxICON_ERROR, this);
        DeleteFileInfoHash(&m_hash);
        delete m_pDrive;
        m_pDrive = NULL;
        GoToStep(STEP_DESTINATION);
        return;
    }

    m_bScanStopped = iResult == ERROR_CANCELLED;
    m_pFilesPage->ShowList(&m_hash);
    m_pFilesPage->SetDriveForPreview(m_pDrive);
    GoToStep(STEP_FILES);
    if (m_hash.empty() && !m_bScanStopped)
        SetHeading(Tr(L"No deleted files found"), wxString::Format(Tr(L"There are no deleted files on %s that could be recovered."), m_source.sName));
}

/////////////////////////////////// the recovery ///////////////////////////////////

void CWizardFrame::StartRecovery() {
    std::vector<t_FileInfo *> vFiles;
    m_pFilesPage->GetList()->GetCheckedFiles(vFiles);
    if (vFiles.empty() || !m_pDrive)
        return;

#ifdef DEMO_BUILD
    wxMessageBox(wxT("This is a DEMO version!\nFile saving is not possible.\nTo buy full version, visit www.getmyfilesback.com"),
            Tr(L"Error"), wxOK | wxICON_ERROR, this);
    return;
#endif

    // overwritten files: other files now use their clusters - warn before spending time on them
    int iOverwritten = 0;
    m_llCopyBytes = 0;
    for (size_t i = 0; i < vFiles.size(); i++) {
        if (vFiles[i]->iRecoverability == RECOVERABILITY_OVERWRITTEN)
            iOverwritten++;
        m_llCopyBytes += vFiles[i]->n64RealSize;
    }
    if (iOverwritten) {
        wxString sQuestion = (iOverwritten == 1)
                ? wxString::Format(Tr(L"1 of the %d ticked files is marked as overwritten: another file is now stored where its data was. It will most likely be unreadable.\n\nRecover it anyway?"),
                (int) vFiles.size())
                : wxString::Format(Tr(L"%d of the %d ticked files are marked as overwritten: other files are now stored where their data was. They will most likely be unreadable.\n\nRecover them anyway?"),
                iOverwritten, (int) vFiles.size());
        if (wxMessageBox(sQuestion, Tr(L"Overwritten files"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING, this) != wxYES)
            return;
    }

    m_pCopy = new CCopyThread(this, m_pDrive, vFiles, m_sDestination);
    if (m_pCopy->Run() != wxTHREAD_NO_ERROR) {
        delete m_pCopy;
        m_pCopy = NULL;
        wxMessageBox(Tr(L"Can't create thread for reading the drive."), Tr(L"Error"), wxOK | wxICON_ERROR, this);
        return;
    }
    // the copy thread owns the drive now - no previews or content checks until it's done
    m_pFilesPage->SetDriveForPreview(NULL);
    SetHeading(Tr(L"Recovering files"), Tr(L"They are copied to the destination folder - nothing is written to the drive they come from."));
    m_pFilesPage->ShowProgress(Tr(L"Recovering files..."), wxEmptyString);
    m_progressTimer.Start(150);
    UpdateFooter();
}

void CWizardFrame::OnCopyDone() {
    m_pCopy->Wait();
    std::vector<t_FileInfo *> vFiles = m_pCopy->GetFiles();
    int iPartly = 0, iOverwritten = 0;
    for (size_t i = 0; i < vFiles.size(); i++) {
        if (vFiles[i]->iRecoverability == RECOVERABILITY_PARTIAL)
            iPartly++;
        else if (vFiles[i]->iRecoverability == RECOVERABILITY_OVERWRITTEN)
            iOverwritten++;
    }
    m_pDonePage->ShowResult(m_pCopy->m_iCopied, (int) vFiles.size(), iPartly, iOverwritten, m_pCopy->m_bCancelled,
            m_pCopy->m_vFailures, m_sDestination);
    delete m_pCopy;
    m_pCopy = NULL;
    m_progressTimer.Stop();
    wxLog::FlushActive();

    m_pFilesPage->SetDriveForPreview(m_pDrive);
    m_pFilesPage->ShowListAgain(); // for Back and "Recover more files"
    GoToStep(STEP_DONE);
}

void CWizardFrame::OnThreadDone(wxThreadEvent &event) {
    if (event.GetId() == THREAD_SCAN_DONE && m_pScan)
        OnScanDone();
    else if (event.GetId() == THREAD_COPY_DONE && m_pCopy)
        OnCopyDone();
}

void CWizardFrame::OnProgressTimer(wxTimerEvent &WXUNUSED(event)) {
    if (m_pScan) {
        m_pFilesPage->SetProgress(m_pScan->GetPercent(), wxEmptyString);
    } else if (m_pCopy) {
        LONGLONG llTotalKB = wxMax(1LL, m_llCopyBytes / 1024);
        LONG lKB = m_pCopy->GetKBWritten();
        int iCurrent = m_pCopy->GetCurrentIndex();
        const std::vector<t_FileInfo *> &vFiles = m_pCopy->GetFiles();
        wxString sNote;
        if (iCurrent >= 0 && iCurrent < (int) vFiles.size())
            sNote = wxString::Format(Tr(L"Recovering %s (%d of %d)"), wxString(vFiles[iCurrent]->sFilename.c_str()), iCurrent + 1, (int) vFiles.size())
                + "\n" + wxString::Format(Tr(L"%s of %s"), FormatFileSize((LONGLONG) lKB * 1024), FormatFileSize(m_llCopyBytes));
        m_pFilesPage->SetProgress((int) wxMin(100LL, (LONGLONG) lKB * 100 / llTotalKB), sNote);
    }
}

void CWizardFrame::StopThreads() {
    wxBusyCursor busy;
    if (m_pScan) {
        m_pScan->Cancel();
        m_pScan->Wait();
        delete m_pScan->m_pDrive;
        delete m_pScan;
        m_pScan = NULL;
    }
    if (m_pCopy) {
        m_pCopy->Cancel();
        m_pCopy->Wait();
        delete m_pCopy;
        m_pCopy = NULL;
    }
}

void CWizardFrame::OnClose(wxCloseEvent &event) {
    // a thread still reading the drive stops first - it uses the engine and the hash this window owns
    StopThreads();
    m_progressTimer.Stop();
    event.Skip();
}
