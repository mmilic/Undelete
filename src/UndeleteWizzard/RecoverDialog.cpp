// RecoverDialog.cpp: where the recovered files go, the copying, and what came of it

#include <windows.h>
#include <shellapi.h> // ShellExecute - not in a WIN32_LEAN_AND_MEAN windows.h
#include <wx/wx.h>
#include <wx/filename.h>

#include "RecoverDialog.h"
#include "DeletedFilesList.h"
#include "MyThreadClass.h"
#include "AppLog.h"
#include "Theme.h"
#include "Volumes.h"
#include "languages/Translate.h"

static wxStaticText *Label(wxWindow *parent, const wxString &sText, const wxFont &font, const wxColour &colour) {
    wxStaticText *pText = new wxStaticText(parent, wxID_ANY, sText);
    pText->SetFont(font);
    pText->SetForegroundColour(colour);
    return pText;
}

static wxString RecoverTitle(int iFiles) {
    return iFiles == 1 ? wxString(Tr(L"Recover 1 file")) : wxString::Format(Tr(L"Recover %d files"), iFiles);
}

CRecoverDialog::CRecoverDialog(wxWindow *parent, CRecoveryDrive *pDrive, const ST_DRIVE &source, const std::vector<t_FileInfo *> &vFiles,
        const wxString &sLastDestination)
: wxDialog(parent, wxID_ANY, "GetMyFilesBack") {
    m_pDrive = pDrive;
    m_source = source;
    m_vFiles = vFiles;
    m_pCopy = NULL;
    m_llBytes = 0;
    for (size_t i = 0; i < vFiles.size(); i++)
        m_llBytes += vFiles[i]->n64RealSize;

    SetBackgroundColour(Theme::PanelBg);
    SetFont(Theme::Body());
    m_pTitle = Label(this, RecoverTitle((int) vFiles.size()), Theme::Title(), Theme::Text);

    wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
    pSizer->Add(m_pTitle, 0, wxBOTTOM, FromDIP(14));
    pSizer->Add(m_pChoose = CreateChoose(), 0, wxEXPAND);
    pSizer->Add(m_pCopying = CreateCopying(), 0, wxEXPAND);
    pSizer->Add(m_pDone = CreateDone(), 0, wxEXPAND);
    pSizer->SetMinSize(FromDIP(wxSize(520, -1)));
    wxBoxSizer *pOuter = new wxBoxSizer(wxVERTICAL);
    pOuter->Add(pSizer, 1, wxEXPAND | wxALL, FromDIP(24));
    SetSizer(pOuter);

    // the last folder, if it is still a good one for this drive - else the best suggestion
    std::vector<wxString> vSuggestions = SuggestDestinations(source.sSource, 3);
    for (size_t i = 0; i < vSuggestions.size(); i++) {
        CFlatButton *pChip = new CFlatButton(m_pChoose, wxID_ANY, vSuggestions[i], FLAT_BUTTON_CHIP);
        pChip->Bind(wxEVT_BUTTON, [this, pChip](wxCommandEvent &) { m_pPath->SetValue(pChip->GetLabel()); });
        m_pChips->Add(pChip, 0, wxRIGHT | wxBOTTOM, FromDIP(8));
    }
    if (!sLastDestination.IsEmpty() && !IsOnSameVolume(sLastDestination, source.sSource))
        m_pPath->ChangeValue(sLastDestination);
    else if (!vSuggestions.empty())
        m_pPath->ChangeValue(vSuggestions[0]);

    m_timer.SetOwner(this);
    Bind(wxEVT_TIMER, &CRecoverDialog::OnTimer, this);
    Bind(wxEVT_THREAD, &CRecoverDialog::OnThreadDone, this);
    Bind(wxEVT_CLOSE_WINDOW, &CRecoverDialog::OnClose, this);
    Bind(wxEVT_BUTTON, &CRecoverDialog::OnCancel, this, wxID_CANCEL); // also Esc

    CheckPath(); // the warning, before the first layout
    ShowPhase(m_pChoose);
    CentreOnParent();
}

CRecoverDialog::~CRecoverDialog() {
    if (m_pCopy) { // can't be - closing waits for the copy - but never leave a thread reading the drive
        m_pCopy->Cancel();
        m_pCopy->Wait();
        delete m_pCopy;
    }
}

wxPanel *CRecoverDialog::CreateChoose() {
    wxPanel *pPanel = new wxPanel(this, wxID_ANY);
    pPanel->SetBackgroundColour(Theme::PanelBg);
    m_pPath = new wxTextCtrl(pPanel, wxID_ANY, wxEmptyString);
    m_pPath->SetFont(Theme::Body());
    m_pPath->SetHint(Tr(L"A folder on another drive"));
    m_pPath->Bind(wxEVT_TEXT, [this](wxCommandEvent &) { CheckPath(); });
    CFlatButton *pBrowse = new CFlatButton(pPanel, wxID_ANY, Tr(L"Browse..."), FLAT_BUTTON_SECONDARY);
    pBrowse->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) {
        wxDirDialog dialog(this, Tr(L"Choose a folder on another drive"), GetPath());
        if (dialog.ShowModal() != wxID_OK)
            return;
        wxFileName folder = wxFileName::DirName(dialog.GetPath());
        folder.AppendDir("RetrievedFiles"); // the files go into a folder of their own, not loose among others
        m_pPath->SetValue(folder.GetPath());
    });
    wxBoxSizer *pRow = new wxBoxSizer(wxHORIZONTAL);
    pRow->Add(m_pPath, 1, wxALIGN_CENTER_VERTICAL);
    pRow->Add(pBrowse, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(8));

    m_pChips = new wxWrapSizer(wxHORIZONTAL, wxREMOVE_LEADING_SPACES);
    m_pWarning = new CBanner(pPanel, BANNER_WARNING);
    m_pWarning->Hide();

    CFlatButton *pCancel = new CFlatButton(pPanel, wxID_CANCEL, Tr(L"Cancel"), FLAT_BUTTON_SECONDARY);
    m_pStart = new CFlatButton(pPanel, wxID_ANY, Tr(L"Recover"), FLAT_BUTTON_PRIMARY);
    m_pStart->Bind(wxEVT_BUTTON, &CRecoverDialog::OnStart, this);
    wxBoxSizer *pButtons = new wxBoxSizer(wxHORIZONTAL);
    pButtons->AddStretchSpacer();
    pButtons->Add(pCancel);
    pButtons->Add(m_pStart, 0, wxLEFT, FromDIP(8));

    wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
    pSizer->Add(Label(pPanel, Tr(L"Save them to"), Theme::Bold(Theme::Body()), Theme::Text), 0, wxBOTTOM, FromDIP(8));
    pSizer->Add(pRow, 0, wxEXPAND | wxBOTTOM, FromDIP(12));
    pSizer->Add(m_pChips, 0, wxEXPAND | wxBOTTOM, FromDIP(4));
    pSizer->Add(m_pWarning, 0, wxEXPAND | wxBOTTOM, FromDIP(8));
    pSizer->Add(pButtons, 0, wxEXPAND | wxTOP, FromDIP(6));
    pPanel->SetSizer(pSizer);
    return pPanel;
}

wxPanel *CRecoverDialog::CreateCopying() {
    wxPanel *pPanel = new wxPanel(this, wxID_ANY);
    pPanel->SetBackgroundColour(Theme::PanelBg);
    m_pCopyText = Label(pPanel, wxEmptyString, Theme::Small(), wxColour(0x3B, 0x3B, 0x3B));
    m_pBar = new CThinBar(pPanel);
    CFlatButton *pCancel = new CFlatButton(pPanel, wxID_CANCEL, Tr(L"Cancel"), FLAT_BUTTON_SECONDARY);
    wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
    pSizer->Add(m_pCopyText, 0, wxEXPAND);
    pSizer->Add(m_pBar, 0, wxEXPAND | wxTOP, FromDIP(10));
    pSizer->Add(pCancel, 0, wxALIGN_RIGHT | wxTOP, FromDIP(18));
    pPanel->SetSizer(pSizer);
    return pPanel;
}

wxPanel *CRecoverDialog::CreateDone() {
    wxPanel *pPanel = new wxPanel(this, wxID_ANY);
    pPanel->SetBackgroundColour(Theme::PanelBg);
    m_pResult = Label(pPanel, wxEmptyString, Theme::Bold(Theme::Body()), Theme::Text);
    m_pFolder = new wxStaticText(pPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(430, -1)),
            wxST_NO_AUTORESIZE | wxST_ELLIPSIZE_MIDDLE);
    m_pFolder->SetFont(Theme::Body());
    m_pFolder->SetForegroundColour(wxColour(0x3B, 0x3B, 0x3B));
    m_pNote = Label(pPanel, wxEmptyString, Theme::Body(), wxColour(0x3B, 0x3B, 0x3B));
    m_pFailuresLabel = Label(pPanel, Tr(L"These files could not be recovered:"), Theme::Small(), Theme::Text);
    m_pFailures = new wxTextCtrl(pPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(-1, 90)),
            wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);
    m_pFailures->SetFont(Theme::Small());

    wxBoxSizer *pTexts = new wxBoxSizer(wxVERTICAL);
    pTexts->Add(m_pResult);
    pTexts->Add(m_pFolder, 0, wxTOP, FromDIP(2));
    pTexts->Add(m_pNote, 0, wxTOP, FromDIP(8));
    wxBoxSizer *pTop = new wxBoxSizer(wxHORIZONTAL);
    pTop->Add(new wxStaticBitmap(pPanel, wxID_ANY, Theme::Icon(Theme::Icons::CheckCircle, Theme::Accent, 28, 1.8)), 0, wxRIGHT, FromDIP(12));
    pTop->Add(pTexts, 1);

    CFlatButton *pClose = new CFlatButton(pPanel, wxID_ANY, Tr(L"Close"), FLAT_BUTTON_SECONDARY);
    CFlatButton *pOpen = new CFlatButton(pPanel, wxID_ANY, Tr(L"Open folder"), FLAT_BUTTON_PRIMARY);
    pClose->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) { EndModal(wxID_OK); });
    pOpen->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) {
        ShellExecuteW(NULL, L"explore", m_sUsedDestination.wc_str(), NULL, NULL, SW_SHOWNORMAL);
    });
    wxBoxSizer *pButtons = new wxBoxSizer(wxHORIZONTAL);
    pButtons->AddStretchSpacer();
    pButtons->Add(pClose);
    pButtons->Add(pOpen, 0, wxLEFT, FromDIP(8));

    wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
    pSizer->Add(pTop, 0, wxEXPAND);
    pSizer->Add(m_pFailuresLabel, 0, wxTOP, FromDIP(14));
    pSizer->Add(m_pFailures, 0, wxEXPAND | wxTOP, FromDIP(6));
    pSizer->Add(pButtons, 0, wxEXPAND | wxTOP, FromDIP(18));
    pPanel->SetSizer(pSizer);
    return pPanel;
}

void CRecoverDialog::ShowPhase(wxPanel *pPhase) {
    m_pChoose->Show(pPhase == m_pChoose);
    m_pCopying->Show(pPhase == m_pCopying);
    m_pDone->Show(pPhase == m_pDone);
    Layout();
    Fit();
}

wxString CRecoverDialog::GetPath() const {
    wxString sPath = m_pPath->GetValue();
    sPath.Trim().Trim(false);
    return sPath;
}

bool CRecoverDialog::IsValid() const {
    wxString sPath = GetPath();
    return !sPath.IsEmpty() && wxIsAbsolutePath(sPath) && !IsOnSameVolume(sPath, m_source.sSource);
}

void CRecoverDialog::CheckPath() {
    wxString sPath = GetPath();
    bool bSameDrive = !sPath.IsEmpty() && IsOnSameVolume(sPath, m_source.sSource);
    if (bSameDrive)
        m_pWarning->SetMessage(wxString::Format(Tr(L"That's %s - the drive you are recovering from."), m_source.sName),
            Tr(L"Saving there can overwrite the files you want back."));
    m_pWarning->Show(bSameDrive);
    m_pStart->Enable(IsValid());
    m_pChoose->Layout();
    if (IsShown()) {
        Layout();
        Fit();
    }
}

/////////////////////////////////// the copying ///////////////////////////////////

void CRecoverDialog::OnStart(wxCommandEvent &WXUNUSED(event)) {
    if (!IsValid())
        return;
    wxString sDestination = GetPath();
    if (!wxFileName::DirExists(sDestination) && !wxFileName::Mkdir(sDestination, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
        wxMessageBox(wxString::Format(Tr(L"The folder %s can't be created."), sDestination), Tr(L"Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    // overwritten files: other files now use their clusters - warn before spending time on them
    int iOverwritten = 0;
    for (size_t i = 0; i < m_vFiles.size(); i++)
        if (m_vFiles[i]->iRecoverability == RECOVERABILITY_OVERWRITTEN)
            iOverwritten++;
    if (iOverwritten) {
        wxString sQuestion = (iOverwritten == 1)
                ? wxString::Format(Tr(L"1 of the %d ticked files is marked as overwritten: another file is now stored where its data was. It will most likely be unreadable.\n\nRecover it anyway?"),
                (int) m_vFiles.size())
                : wxString::Format(Tr(L"%d of the %d ticked files are marked as overwritten: other files are now stored where their data was. They will most likely be unreadable.\n\nRecover them anyway?"),
                iOverwritten, (int) m_vFiles.size());
        if (wxMessageBox(sQuestion, Tr(L"Overwritten files"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING, this) != wxYES)
            return;
    }

    // the log goes to a file in the destination - what was logged so far (the scan) goes there too
    CAppLog::Get()->OpenFile(sDestination);
    wxLogMessage("Source: %s (%s), destination: %s", m_source.sSource, m_source.sFileSystem, sDestination);

    m_pCopy = new CCopyThread(this, m_pDrive, m_vFiles, sDestination);
    if (m_pCopy->Run() != wxTHREAD_NO_ERROR) {
        delete m_pCopy;
        m_pCopy = NULL;
        wxMessageBox(Tr(L"Can't create thread for reading the drive."), Tr(L"Error"), wxOK | wxICON_ERROR, this);
        return;
    }
    m_sUsedDestination = sDestination;
    m_pCopyText->SetLabel(Tr(L"Recovering files..."));
    m_pBar->SetValue(0);
    ShowPhase(m_pCopying);
    m_timer.Start(150);
}

void CRecoverDialog::OnTimer(wxTimerEvent &WXUNUSED(event)) {
    if (!m_pCopy)
        return;
    LONGLONG llTotalKB = wxMax(1LL, m_llBytes / 1024);
    LONG lKB = m_pCopy->GetKBWritten();
    int iCurrent = m_pCopy->GetCurrentIndex();
    if (iCurrent >= 0 && iCurrent < (int) m_vFiles.size())
        m_pCopyText->SetLabel(wxString::Format(Tr(L"Recovering %s (%d of %d)"), wxString(m_vFiles[iCurrent]->sFilename.c_str()),
            iCurrent + 1, (int) m_vFiles.size()) + L" · "
            + wxString::Format(Tr(L"%s of %s"), FormatFileSize((LONGLONG) lKB * 1024), FormatFileSize(m_llBytes)));
    m_pBar->SetValue((int) wxMin(100LL, (LONGLONG) lKB * 100 / llTotalKB));
}

void CRecoverDialog::OnThreadDone(wxThreadEvent &WXUNUSED(event)) {
    if (!m_pCopy)
        return;
    m_pCopy->Wait();
    m_timer.Stop();
    wxLog::FlushActive();
    int iCopied = m_pCopy->m_iCopied;
    bool bCancelled = m_pCopy->m_bCancelled;
    std::vector<wxString> vFailures = m_pCopy->m_vFailures;
    delete m_pCopy;
    m_pCopy = NULL;

    int iPartly = 0, iOverwritten = 0;
    for (size_t i = 0; i < m_vFiles.size(); i++) {
        if (m_vFiles[i]->iRecoverability == RECOVERABILITY_PARTIAL)
            iPartly++;
        else if (m_vFiles[i]->iRecoverability == RECOVERABILITY_OVERWRITTEN)
            iOverwritten++;
    }

    m_pTitle->SetLabel(bCancelled ? Tr(L"Recovery stopped") : Tr(L"Recovery finished"));
    if (bCancelled)
        m_pResult->SetLabel(wxString::Format(Tr(L"Recovery stopped - %d of %d files recovered"), iCopied, (int) m_vFiles.size()));
    else
        m_pResult->SetLabel(iCopied == 1 ? wxString(Tr(L"1 file recovered")) : wxString::Format(Tr(L"%d files recovered"), iCopied));
    m_pFolder->SetLabel(wxString::Format(Tr(L"Saved in %s"), m_sUsedDestination));
    m_pFolder->SetToolTip(m_sUsedDestination);

    wxString sNote;
    if (iOverwritten)
        sNote = wxString::Format(Tr(L"%d of them were marked as overwritten - other files are stored where their data was, so they will most likely be unreadable."), iOverwritten);
    else if (iPartly)
        sNote = wxString::Format(Tr(L"%d of them were partly overwritten and may not open completely."), iPartly);
    else if (iCopied)
        sNote = Tr(L"All of them were intact on the drive.");
    m_pNote->SetLabel(sNote);
    m_pNote->Wrap(FromDIP(430));

    wxString sFailures;
    for (size_t i = 0; i < vFailures.size(); i++)
        sFailures += vFailures[i] + "\n";
    m_pFailures->SetValue(sFailures);
    m_pFailuresLabel->Show(!vFailures.empty());
    m_pFailures->Show(!vFailures.empty());
    ShowPhase(m_pDone);
}

// Cancel, Esc, the close box: before the copying - close; during it - stop it, the result follows

void CRecoverDialog::OnCancel(wxCommandEvent &WXUNUSED(event)) {
    if (m_pCopy)
        m_pCopy->Cancel();
    else
        EndModal(wxID_CANCEL);
}

void CRecoverDialog::OnClose(wxCloseEvent &event) {
    if (m_pCopy) {
        m_pCopy->Cancel();
        if (event.CanVeto()) {
            event.Veto();
            return;
        }
        m_pCopy->Wait();
    }
    event.Skip();
}
