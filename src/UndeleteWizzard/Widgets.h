/*
 * File:   Widgets.h
 *
 * The design's controls that Windows doesn't have: flat buttons in the accent colour, a segmented filter,
 * banners, the step list, drive cards and a thin progress bar. All drawn by hand, sized in DIPs, so they
 * look the same at any DPI. Clicks arrive as ordinary wxEVT_BUTTON / wxEVT_CHOICE events.
 */

#ifndef WIDGETS_H
#define	WIDGETS_H

#include <wx/wx.h>
#include <vector>

// a control drawn by hand: no background erasing, the parent's colour around rounded corners

class CPaintedControl : public wxControl {
public:
    CPaintedControl(wxWindow *parent, wxWindowID id, long style = wxBORDER_NONE);
    bool AcceptsFocus() const override { return IsEnabled() && m_bFocusable; }
    bool Enable(bool bEnable = true) override;

protected:
    bool m_bHover;
    bool m_bPressed;
    bool m_bFocusable;

    virtual void Paint(wxDC &dc) = 0; // a wxGCDC - anti-aliased
    virtual void OnClick() { SendClick(); }
    void SendClick();
    wxColour ParentBackground() const;

private:
    void OnPaint(wxPaintEvent &event);
    void OnMouse(wxMouseEvent &event);
    void OnKey(wxKeyEvent &event);
    void OnFocus(wxFocusEvent &event);
    void OnCaptureLost(wxMouseCaptureLostEvent &event);
};

enum {
    FLAT_BUTTON_PRIMARY, // accent fill, white text - the one action that moves on
    FLAT_BUTTON_SECONDARY, // white, grey outline
    FLAT_BUTTON_CHIP // small and round, for suggestions
};

class CFlatButton : public CPaintedControl {
public:
    CFlatButton(wxWindow *parent, wxWindowID id, const wxString &sLabel, int iStyle);
    void SetLabel(const wxString &sLabel) override;

protected:
    void Paint(wxDC &dc) override;
    wxSize DoGetBestClientSize() const override;

private:
    int m_iStyle;
};

// "All files | Good only | Hide overwritten" - one of them pressed

class CSegmented : public CPaintedControl {
public:
    CSegmented(wxWindow *parent, wxWindowID id, const std::vector<wxString> &vOptions);
    int GetSelection() const { return m_iSelection; }
    void SetSelection(int iSelection);

protected:
    void Paint(wxDC &dc) override;
    wxSize DoGetBestClientSize() const override;
    void OnClick() override;

private:
    std::vector<wxString> m_vOptions;
    int m_iSelection;
    int m_iClicked; // the segment under the mouse when it was pressed
    std::vector<wxRect> SegmentRects() const;
    void OnLeftDown(wxMouseEvent &event);
    void OnKeyDown(wxKeyEvent &event);
};

// a coloured box with an icon and a message - a warning, or a confirmation

enum {
    BANNER_WARNING,
    BANNER_INFO
};

class CBanner : public wxPanel {
public:
    CBanner(wxWindow *parent, int iKind);
    // sTitle is bold, sText follows it
    void SetMessage(const wxString &sTitle, const wxString &sText);

private:
    int m_iKind;
    wxStaticText *m_pTitle;
    wxStaticText *m_pText;
    wxString m_sTitle, m_sText;
    void OnPaint(wxPaintEvent &event);
    void OnSize(wxSizeEvent &event);
};

// the wizard's steps down the left side: done (tick), current (filled), still to come

class CStepList : public wxWindow {
public:
    CStepList(wxWindow *parent, const std::vector<wxString> &vSteps);
    void SetCurrent(int iStep); // 0-based

protected:
    wxSize DoGetBestClientSize() const override;

private:
    std::vector<wxString> m_vSteps;
    int m_iCurrent;
    void OnPaint(wxPaintEvent &event);
};

// a drive to scan

struct ST_DRIVE {
    wxString sSource; // "E:\" - or the path of a disk image
    wxString sName; // "USB stick (E:)"
    wxString sFileSystem; // "NTFS", "FAT32", "exFAT"...
    ULONGLONG ullTotal;
    ULONGLONG ullFree;
    bool bRemovable;
    bool bImage;
    bool bSupported; // a file system the engine reads
    wxString sNote; // why it can't be scanned
};

// the drives that can hold deleted files: hard disks and USB drives with a file system Windows can read
std::vector<ST_DRIVE> ListDrives();
// a disk image file as a drive (the test images, a dd copy of a failing disk...)
ST_DRIVE DiskImageDrive(const wxString &sPath);

class CDriveCard : public CPaintedControl {
public:
    // bCompact: one row high, for the main window's drive strip
    CDriveCard(wxWindow *parent, wxWindowID id, const ST_DRIVE &drive, bool bCompact);
    const ST_DRIVE &GetDrive() const { return m_drive; }
    void SetSelected(bool bSelected);
    bool IsSelected() const { return m_bSelected; }

protected:
    void Paint(wxDC &dc) override;
    wxSize DoGetBestClientSize() const override;

private:
    ST_DRIVE m_drive;
    bool m_bCompact;
    bool m_bSelected;
    wxBitmapBundle m_icon;
};

// "930 GB", "3.7 TB" - 1024-based, like Explorer
wxString FormatDriveSize(ULONGLONG ullBytes);

// a 6 px bar: a value 0..100, or an indeterminate sliding segment

class CThinBar : public wxWindow {
public:
    CThinBar(wxWindow *parent, int iHeight = 6);
    void SetValue(int iPercent); // -1: indeterminate
    void SetColour(const wxColour &colour);

protected:
    wxSize DoGetBestClientSize() const override;

private:
    int m_iHeight;
    int m_iValue;
    int m_iPhase; // where the indeterminate segment is
    wxColour m_colour;
    wxTimer m_timer;
    void OnPaint(wxPaintEvent &event);
    void OnTimer(wxTimerEvent &event);
};

#endif	/* WIDGETS_H */
