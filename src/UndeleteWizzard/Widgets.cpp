// Widgets.cpp: the hand-drawn controls of the design

#include <windows.h>
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/file.h>
#include <wx/filename.h>
#include "Widgets.h"
#include "Theme.h"
#include "languages/Translate.h"

/////////////////////////////////// CPaintedControl ///////////////////////////////////

CPaintedControl::CPaintedControl(wxWindow *parent, wxWindowID id, long style)
: wxControl(parent, id, wxDefaultPosition, wxDefaultSize, style) {
    m_bHover = false;
    m_bPressed = false;
    m_bFocusable = true;
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetFont(Theme::Body());

    Bind(wxEVT_PAINT, &CPaintedControl::OnPaint, this);
    Bind(wxEVT_ENTER_WINDOW, &CPaintedControl::OnMouse, this);
    Bind(wxEVT_LEAVE_WINDOW, &CPaintedControl::OnMouse, this);
    Bind(wxEVT_LEFT_DOWN, &CPaintedControl::OnMouse, this);
    Bind(wxEVT_LEFT_DCLICK, &CPaintedControl::OnMouse, this);
    Bind(wxEVT_LEFT_UP, &CPaintedControl::OnMouse, this);
    Bind(wxEVT_KEY_DOWN, &CPaintedControl::OnKey, this);
    Bind(wxEVT_SET_FOCUS, &CPaintedControl::OnFocus, this);
    Bind(wxEVT_KILL_FOCUS, &CPaintedControl::OnFocus, this);
    Bind(wxEVT_MOUSE_CAPTURE_LOST, &CPaintedControl::OnCaptureLost, this);
}

bool CPaintedControl::Enable(bool bEnable) {
    bool bChanged = wxControl::Enable(bEnable);
    if (bChanged) {
        m_bHover = m_bPressed = false;
        Refresh();
    }
    return bChanged;
}

wxColour CPaintedControl::ParentBackground() const {
    for (wxWindow *p = GetParent(); p; p = p->GetParent())
        if (p->UseBgCol() || p->IsTopLevel())
            return p->GetBackgroundColour();
    return Theme::PanelBg;
}

void CPaintedControl::SendClick() {
    wxCommandEvent event(wxEVT_BUTTON, GetId());
    event.SetEventObject(this);
    ProcessWindowEvent(event);
}

void CPaintedControl::OnPaint(wxPaintEvent &WXUNUSED(event)) {
    wxAutoBufferedPaintDC dcPaint(this);
    dcPaint.SetBackground(wxBrush(ParentBackground()));
    dcPaint.Clear();
    wxGCDC dc(dcPaint);
    dc.SetFont(GetFont());
    Paint(dc);
}

void CPaintedControl::OnMouse(wxMouseEvent &event) {
    event.Skip();
    if (!IsEnabled())
        return;
    if (event.Entering()) {
        m_bHover = true;
    } else if (event.Leaving()) {
        m_bHover = false;
    } else if (event.LeftDown() || event.LeftDClick()) {
        m_bPressed = true;
        if (AcceptsFocus())
            SetFocus();
        if (!HasCapture())
            CaptureMouse();
    } else if (event.LeftUp()) {
        bool bClick = m_bPressed && GetClientRect().Contains(event.GetPosition());
        m_bPressed = false;
        if (HasCapture())
            ReleaseMouse();
        Refresh();
        if (bClick)
            OnClick();
        return;
    }
    Refresh();
}

void CPaintedControl::OnKey(wxKeyEvent &event) {
    if (IsEnabled() && (event.GetKeyCode() == WXK_SPACE || event.GetKeyCode() == WXK_RETURN || event.GetKeyCode() == WXK_NUMPAD_ENTER)) {
        OnClick();
        return;
    }
    event.Skip();
}

void CPaintedControl::OnFocus(wxFocusEvent &event) {
    Refresh();
    event.Skip();
}

void CPaintedControl::OnCaptureLost(wxMouseCaptureLostEvent &WXUNUSED(event)) {
    m_bPressed = false;
    Refresh();
}

// the keyboard focus: a dotted outline just inside the control

static void DrawFocusRing(wxDC &dc, const wxRect &rect, double dRadius) {
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(Theme::Text, 1, wxPENSTYLE_DOT));
    dc.DrawRoundedRectangle(rect, dRadius);
}

/////////////////////////////////// CFlatButton ///////////////////////////////////

CFlatButton::CFlatButton(wxWindow *parent, wxWindowID id, const wxString &sLabel, int iStyle)
: CPaintedControl(parent, id) {
    m_iStyle = iStyle;
    wxControl::SetLabel(sLabel);
    if (iStyle == FLAT_BUTTON_PRIMARY)
        SetFont(Theme::Bold(Theme::Body()));
    else if (iStyle == FLAT_BUTTON_CHIP)
        SetFont(Theme::Small());
    SetCursor(wxCursor(wxCURSOR_HAND));
}

void CFlatButton::SetLabel(const wxString &sLabel) {
    if (sLabel == GetLabel())
        return;
    wxControl::SetLabel(sLabel);
    InvalidateBestSize();
    if (GetParent())
        GetParent()->Layout();
    Refresh();
}

wxSize CFlatButton::DoGetBestClientSize() const {
    wxClientDC dc(const_cast<CFlatButton *> (this));
    dc.SetFont(GetFont());
    wxSize text = dc.GetTextExtent(wxStripMenuCodes(GetLabel()));
    if (m_iStyle == FLAT_BUTTON_CHIP)
        return wxSize(text.x + FromDIP(24), FromDIP(32));
    int iWidth = text.x + FromDIP(m_iStyle == FLAT_BUTTON_PRIMARY ? 36 : 32);
    return wxSize(wxMax(iWidth, FromDIP(96)), FromDIP(40));
}

void CFlatButton::Paint(wxDC &dc) {
    wxRect rect = GetClientRect();
    double dRadius = (m_iStyle == FLAT_BUTTON_CHIP) ? rect.height / 2.0 : FromDIP(6);
    bool bEnabled = IsEnabled();
    wxColour fill, border, text;

    if (m_iStyle == FLAT_BUTTON_PRIMARY) {
        fill = bEnabled ? (m_bPressed || m_bHover ? Theme::AccentHover : Theme::Accent) : wxColour(0x9F, 0xBF, 0xE0);
        border = fill;
        text = *wxWHITE;
    } else {
        fill = (m_iStyle == FLAT_BUTTON_CHIP) ? wxColour(0xFA, 0xFA, 0xFA) : Theme::PanelBg;
        if (bEnabled && (m_bHover || m_bPressed))
            fill = m_bPressed ? wxColour(0xEB, 0xEB, 0xEB) : wxColour(0xF3, 0xF3, 0xF3);
        border = Theme::ControlBorder;
        text = bEnabled ? Theme::Text : Theme::TextDisabled;
    }
    Theme::DrawRoundedBox(dc, rect, dRadius, fill, border);
    if (HasFocus())
        DrawFocusRing(dc, rect.Deflate(FromDIP(3)), dRadius - FromDIP(2));

    dc.SetTextForeground(text);
    wxString sLabel = wxStripMenuCodes(GetLabel());
    wxSize extent = dc.GetTextExtent(sLabel);
    dc.DrawText(sLabel, rect.x + (rect.width - extent.x) / 2, rect.y + (rect.height - extent.y) / 2);
}

/////////////////////////////////// CBanner ///////////////////////////////////

CBanner::CBanner(wxWindow *parent, int iKind)
: wxPanel(parent, wxID_ANY) {
    m_iKind = iKind;
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(iKind == BANNER_WARNING ? Theme::WarningBg : Theme::InfoBg);
    wxColour text = (iKind == BANNER_WARNING) ? Theme::WarningText : Theme::Text;

    wxStaticBitmap *pIcon = new wxStaticBitmap(this, wxID_ANY, iKind == BANNER_WARNING
            ? Theme::Icon(Theme::Icons::Warning, Theme::WarningIcon, 20, 1.8) : Theme::Icon(Theme::Icons::Check, Theme::Accent, 20, 1.8));
    m_pTitle = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_pTitle->SetFont(Theme::Bold(Theme::Body()));
    m_pTitle->SetForegroundColour(text);
    m_pText = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_pText->SetFont(Theme::Body());
    m_pText->SetForegroundColour(text);

    wxBoxSizer *pTexts = new wxBoxSizer(wxVERTICAL);
    pTexts->Add(m_pTitle, 0, wxEXPAND);
    pTexts->Add(m_pText, 0, wxEXPAND | wxTOP, FromDIP(2));
    wxBoxSizer *pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(pIcon, 0, wxALL, FromDIP(12));
    pSizer->Add(pTexts, 1, wxTOP | wxBOTTOM | wxRIGHT, FromDIP(12));
    SetSizer(pSizer);

    Bind(wxEVT_PAINT, &CBanner::OnPaint, this);
    Bind(wxEVT_SIZE, &CBanner::OnSize, this);
}

void CBanner::SetMessage(const wxString &sTitle, const wxString &sText) {
    m_sTitle = sTitle;
    m_sText = sText;
    m_pTitle->SetLabel(sTitle);
    m_pText->SetLabel(sText);
    m_pTitle->Show(!sTitle.IsEmpty());
    m_pText->Show(!sText.IsEmpty());
    wxSizeEvent event(GetSize());
    OnSize(event);
}

// the texts wrap at the banner's width - their height then follows

void CBanner::OnSize(wxSizeEvent &event) {
    int iWidth = GetClientSize().x - FromDIP(12 + 20 + 12 + 12);
    if (iWidth > 50) {
        m_pTitle->SetLabel(m_sTitle);
        m_pTitle->Wrap(iWidth);
        m_pText->SetLabel(m_sText);
        m_pText->Wrap(iWidth);
        // the new height has to reach the sizer this banner is in
        wxSize best = GetSizer()->GetMinSize();
        if (best.y != GetMinSize().y) {
            SetMinSize(wxSize(-1, best.y));
            if (GetParent())
                GetParent()->Layout();
        }
    }
    event.Skip();
}

void CBanner::OnPaint(wxPaintEvent &WXUNUSED(event)) {
    wxAutoBufferedPaintDC dcPaint(this);
    wxColour parent = GetParent()->GetBackgroundColour();
    dcPaint.SetBackground(wxBrush(parent));
    dcPaint.Clear();
    wxGCDC dc(dcPaint);
    Theme::DrawRoundedBox(dc, GetClientRect(), FromDIP(6), GetBackgroundColour(),
            m_iKind == BANNER_WARNING ? Theme::WarningBorder : wxNullColour);
}

/////////////////////////////////// drives ///////////////////////////////////

wxString FormatDriveSize(ULONGLONG ullBytes) {
    double d = (double) ullBytes;
    const char *aUnits[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    while (d >= 1024 && i < 4) {
        d /= 1024;
        i++;
    }
    return (d < 10 && i > 0) ? wxString::Format("%.1f %s", d, aUnits[i]) : wxString::Format("%.0f %s", d, aUnits[i]);
}

static bool IsSupportedFileSystem(const wxString &sFileSystem) {
    return sFileSystem == "NTFS" || sFileSystem == "FAT" || sFileSystem == "FAT32" || sFileSystem == "FAT12" || sFileSystem == "FAT16"
            || sFileSystem.CmpNoCase("exFAT") == 0;
}

static wxString UnsupportedNote(const wxString &sFileSystem) {
    if (sFileSystem.IsEmpty())
        return Tr(L"The drive can't be read");
    return wxString::Format(Tr(L"%s drives can't be scanned"), sFileSystem);
}

std::vector<ST_DRIVE> ListDrives() {
    std::vector<ST_DRIVE> vDrives;

    // a card reader without a card must not bring up a "no disk in the drive" message box
    DWORD dwOldErrorMode;
    SetThreadErrorMode(SEM_FAILCRITICALERRORS, &dwOldErrorMode);

    wchar_t wszDrives[512];
    DWORD dwLen = GetLogicalDriveStringsW(511, wszDrives);
    for (wchar_t *pDrive = wszDrives; dwLen && dwLen < 512 && *pDrive; pDrive += wcslen(pDrive) + 1) {
        UINT uType = GetDriveTypeW(pDrive);
        if (uType != DRIVE_FIXED && uType != DRIVE_REMOVABLE)
            continue; // CDs and network drives: no deleted files to get back from those

        ST_DRIVE drive;
        drive.sSource = pDrive;
        drive.bRemovable = uType == DRIVE_REMOVABLE;
        drive.bImage = false;
        drive.ullTotal = drive.ullFree = 0;

        wchar_t wszLabel[MAX_PATH + 1] = L"", wszFileSystem[MAX_PATH + 1] = L"";
        if (!GetVolumeInformationW(pDrive, wszLabel, MAX_PATH + 1, NULL, NULL, NULL, wszFileSystem, MAX_PATH + 1)) {
            if (drive.bRemovable && GetLastError() == ERROR_NOT_READY)
                continue; // an empty card reader
        }
        drive.sFileSystem = wszFileSystem;
        ULARGE_INTEGER ulFree, ulTotal;
        if (GetDiskFreeSpaceExW(pDrive, NULL, &ulTotal, &ulFree)) {
            drive.ullTotal = ulTotal.QuadPart;
            drive.ullFree = ulFree.QuadPart;
        }

        wxString sLetter = wxString(pDrive).Left(2); // "E:"
        wxString sLabel(wszLabel);
        if (sLabel.IsEmpty())
            sLabel = drive.bRemovable ? Tr(L"USB Drive") : Tr(L"Local Disk");
        drive.sName = sLabel + " (" + sLetter + ")";
        drive.bSupported = IsSupportedFileSystem(drive.sFileSystem);
        if (!drive.bSupported)
            drive.sNote = UnsupportedNote(drive.sFileSystem);
        vDrives.push_back(drive);
    }
    SetThreadErrorMode(dwOldErrorMode, NULL);
    return vDrives;
}

ST_DRIVE DiskImageDrive(const wxString &sPath) {
    ST_DRIVE drive;
    drive.sSource = sPath;
    drive.sName = wxFileName(sPath).GetFullName();
    drive.bRemovable = false;
    drive.bImage = true;
    drive.ullTotal = drive.ullFree = 0;
    drive.bSupported = true;

    // the file system, from the boot sector - the engine checks it properly when it opens the image
    wxFile file;
    unsigned char uchBoot[512] = {0};
    if (file.Open(sPath) && file.Read(uchBoot, sizeof (uchBoot)) == sizeof (uchBoot)) {
        drive.ullTotal = (ULONGLONG) file.Length();
        if (!memcmp(uchBoot + 3, "NTFS    ", 8))
            drive.sFileSystem = "NTFS";
        else if (!memcmp(uchBoot + 3, "EXFAT   ", 8))
            drive.sFileSystem = "exFAT";
        else if (!memcmp(uchBoot + 82, "FAT32", 5))
            drive.sFileSystem = "FAT32";
        else if (!memcmp(uchBoot + 54, "FAT", 3))
            drive.sFileSystem = "FAT";
        else
            drive.sFileSystem = Tr(L"Disk image");
    } else {
        drive.sFileSystem = Tr(L"Disk image");
        drive.bSupported = false;
        drive.sNote = Tr(L"The drive can't be read");
    }
    return drive;
}

/////////////////////////////////// CDriveCard ///////////////////////////////////

CDriveCard::CDriveCard(wxWindow *parent, wxWindowID id, const ST_DRIVE &drive)
: CPaintedControl(parent, id) {
    m_drive = drive;
    m_bSelected = false;
    const char *szIcon = drive.bImage ? Theme::Icons::DiskImage : (drive.bRemovable ? Theme::Icons::UsbDrive : Theme::Icons::HardDisk);
    m_icon = Theme::Icon(szIcon, drive.bSupported ? wxColour(0x44, 0x44, 0x44) : Theme::TextDisabled, 26, 1.5);
    SetFont(Theme::Small());
    SetCursor(drive.bSupported ? wxCursor(wxCURSOR_HAND) : wxNullCursor);
    if (!drive.bSupported) {
        m_bFocusable = false;
        SetToolTip(drive.sNote);
    }
}

void CDriveCard::SetSelected(bool bSelected) {
    if (bSelected != m_bSelected) {
        m_bSelected = bSelected;
        Refresh();
    }
}

// "NTFS · 930 GB" - or why it can't be scanned
static wxString DriveMeta(const ST_DRIVE &drive) {
    wxString sMeta = drive.sFileSystem;
    if (!drive.bSupported)
        return sMeta + L" · " + drive.sNote;
    if (drive.ullTotal)
        sMeta += L" · " + FormatDriveSize(drive.ullTotal);
    return sMeta;
}

wxSize CDriveCard::DoGetBestClientSize() const {
    wxClientDC dc(const_cast<CDriveCard *> (this));
    dc.SetFont(Theme::Bold(GetFont()));
    int iText = dc.GetTextExtent(m_drive.sName).x;
    dc.SetFont(GetFont().Scaled(12.0f / 13));
    iText = wxMax(iText, dc.GetTextExtent(DriveMeta(m_drive)).x);
    iText = wxMax(iText, FromDIP(130)); // the usage bar
    return wxSize(wxMin(FromDIP(14 + 26 + 10 + 14) + iText, FromDIP(320)), FromDIP(64));
}

void CDriveCard::Paint(wxDC &dc) {
    wxRect rect = GetClientRect();
    bool bEnabled = m_drive.bSupported;
    wxColour border = m_bSelected ? Theme::Accent : (m_bHover && bEnabled ? wxColour(0xA0, 0xA0, 0xA0) : wxColour(0xD1, 0xD1, 0xD1));
    Theme::DrawRoundedBox(dc, rect, FromDIP(8), bEnabled ? Theme::PanelBg : wxColour(0xFA, 0xFA, 0xFA), border, m_bSelected ? FromDIP(2) : 1);
    if (HasFocus())
        DrawFocusRing(dc, rect.Deflate(FromDIP(4)), FromDIP(5));

    wxFont bold = Theme::Bold(GetFont());
    wxFont small = GetFont().Scaled(12.0f / 13);
    wxString sMeta = DriveMeta(m_drive);
    int iUsed = (m_drive.ullTotal && !m_drive.bImage && bEnabled) ? (int) ((m_drive.ullTotal - m_drive.ullFree) * 100 / m_drive.ullTotal) : -1;

    int iPad = FromDIP(14);
    wxBitmap icon = m_icon.GetBitmapFor(this);
    dc.DrawBitmap(icon, rect.x + iPad, rect.y + (rect.height - icon.GetLogicalHeight()) / 2, true);

    int x = rect.x + iPad + icon.GetLogicalWidth() + FromDIP(10);
    int iTextWidth = rect.GetRight() - iPad - x;
    dc.SetClippingRegion(wxRect(x, rect.y, iTextWidth, rect.height));

    // name, file system and size, how full - centred as one block
    wxCoord iNameHeight, iMetaHeight;
    dc.SetFont(bold);
    dc.GetTextExtent(m_drive.sName, NULL, &iNameHeight);
    dc.SetFont(small);
    dc.GetTextExtent(sMeta, NULL, &iMetaHeight);
    int iBar = FromDIP(4), iGap = FromDIP(3);
    int iBlock = iNameHeight + iGap + iMetaHeight + (iUsed >= 0 ? iGap + iBar : 0);
    int y = rect.y + (rect.height - iBlock) / 2;

    dc.SetFont(bold);
    dc.SetTextForeground(bEnabled ? Theme::Text : Theme::TextDisabled);
    dc.DrawText(m_drive.sName, x, y);
    y += iNameHeight + iGap;
    dc.SetFont(small);
    dc.SetTextForeground(bEnabled ? Theme::TextSecondary : Theme::Partial);
    dc.DrawText(sMeta, x, y);
    y += iMetaHeight + iGap;

    if (iUsed >= 0) {
        wxRect bar(x, y, wxMin(FromDIP(130), iTextWidth), iBar);
        Theme::DrawRoundedBox(dc, bar, iBar / 2.0, Theme::Border, wxNullColour);
        bar.width = bar.width * iUsed / 100;
        if (bar.width > 0)
            Theme::DrawRoundedBox(dc, bar, iBar / 2.0, iUsed > 80 ? Theme::UsageHigh : Theme::Accent, wxNullColour);
    }
    dc.DestroyClippingRegion();
}

/////////////////////////////////// CThinBar ///////////////////////////////////

CThinBar::CThinBar(wxWindow *parent, int iHeight)
: wxWindow(parent, wxID_ANY) {
    m_iHeight = iHeight;
    m_iValue = 0;
    m_iPhase = 0;
    m_colour = Theme::Accent;
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    m_timer.SetOwner(this);
    Bind(wxEVT_PAINT, &CThinBar::OnPaint, this);
    Bind(wxEVT_TIMER, &CThinBar::OnTimer, this);
}

wxSize CThinBar::DoGetBestClientSize() const {
    return wxSize(FromDIP(120), FromDIP(m_iHeight));
}

void CThinBar::SetValue(int iPercent) {
    if (iPercent == m_iValue)
        return;
    m_iValue = iPercent;
    if (iPercent < 0 && !m_timer.IsRunning())
        m_timer.Start(30);
    else if (iPercent >= 0 && m_timer.IsRunning())
        m_timer.Stop();
    Refresh();
}

void CThinBar::SetColour(const wxColour &colour) {
    m_colour = colour;
    Refresh();
}

void CThinBar::OnTimer(wxTimerEvent &WXUNUSED(event)) {
    m_iPhase = (m_iPhase + 2) % 140;
    Refresh();
}

void CThinBar::OnPaint(wxPaintEvent &WXUNUSED(event)) {
    wxAutoBufferedPaintDC dcPaint(this);
    dcPaint.SetBackground(wxBrush(GetParent()->GetBackgroundColour()));
    dcPaint.Clear();
    wxGCDC dc(dcPaint);
    wxRect rect = GetClientRect();
    double dRadius = rect.height / 2.0;
    Theme::DrawRoundedBox(dc, rect, dRadius, Theme::Border, wxNullColour);

    wxRect fill(rect);
    if (m_iValue < 0) { // a third of the bar sliding across
        fill.width = rect.width * 30 / 100;
        fill.x = rect.x + (rect.width + fill.width) * m_iPhase / 140 - fill.width;
        dc.SetClippingRegion(rect);
    } else {
        fill.width = rect.width * wxMin(m_iValue, 100) / 100;
    }
    if (fill.width > 0)
        Theme::DrawRoundedBox(dc, fill, dRadius, m_colour, wxNullColour);
    if (m_iValue < 0)
        dc.DestroyClippingRegion();
}
