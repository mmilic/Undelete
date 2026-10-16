// Theme.cpp: colours, fonts and icons of the design

#include <wx/wx.h>
#include <wx/fontenum.h>
#include "Theme.h"
#include "DeletedFilesList.h"

namespace Theme {
    const wxColour Accent(0x00, 0x5F, 0xB8);
    const wxColour AccentHover(0x00, 0x4A, 0x8F);
    const wxColour WindowBg(0xF3, 0xF3, 0xF3);
    const wxColour PanelBg(0xFF, 0xFF, 0xFF);
    const wxColour SubtleBg(0xF7, 0xF7, 0xF7);
    const wxColour FooterBg(0xF9, 0xF9, 0xF9);
    const wxColour Border(0xE5, 0xE5, 0xE5);
    const wxColour ControlBorder(0xC8, 0xC8, 0xC8);
    const wxColour Text(0x1B, 0x1B, 0x1B);
    const wxColour TextSecondary(0x5C, 0x5C, 0x5C);
    const wxColour TextDisabled(0x8A, 0x8A, 0x8A);
    const wxColour Good(0x0E, 0x6B, 0x0E);
    const wxColour GoodDot(0x13, 0xA1, 0x0E);
    const wxColour Partial(0x8A, 0x53, 0x00);
    const wxColour PartialDot(0xE0, 0xA1, 0x00);
    const wxColour Overwritten(0x61, 0x61, 0x61);
    const wxColour OverwrittenDot(0xB0, 0xB0, 0xB0);
    const wxColour Damaged(0xA4, 0x26, 0x2C);
    const wxColour UsageHigh(0xC2, 0x7C, 0x0E);
    const wxColour WarningBg(0xFF, 0xF4, 0xCE);
    const wxColour WarningBorder(0xE0, 0xB3, 0x00);
    const wxColour WarningText(0x4D, 0x3A, 0x00);
    const wxColour WarningIcon(0x8A, 0x53, 0x00);
    const wxColour InfoBg(0xF0, 0xF6, 0xFC);
    const wxColour RowFocused(0xE1, 0xED, 0xFA);
    const wxColour RowChecked(0xF4, 0xF8, 0xFD);

    // Segoe UI Variable on Windows 11, Segoe UI before it
    static wxFont BaseFont(double dPixels) {
        static wxString s_sFace;
        if (s_sFace.IsEmpty())
            s_sFace = wxFontEnumerator::IsValidFacename("Segoe UI Variable Text") ? "Segoe UI Variable Text" : "Segoe UI";
        wxFont font(wxFontInfo(dPixels * 72.0 / 96.0).FaceName(s_sFace));
        return font;
    }

    wxFont Body() {
        return BaseFont(14);
    }

    wxFont Small() {
        return BaseFont(13);
    }

    wxFont Tiny() {
        return BaseFont(12);
    }

    wxFont Heading() {
        return Bold(BaseFont(22));
    }

    wxFont Title() {
        return Bold(BaseFont(20));
    }

    wxFont Bold(const wxFont &font) {
        wxFont bold(font);
        bold.SetWeight(wxFONTWEIGHT_SEMIBOLD);
        return bold;
    }

    namespace Icons {
        const char *const App = "M6 3h8l4 4v14H6z M14 3v4h4 M12 17v-6 M9.5 13.5L12 11l2.5 2.5";
        const char *const HardDisk = "M4 6h16a1 1 0 0 1 1 1v10a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1V7a1 1 0 0 1 1-1z M3 13h18 M16.5 15.5h1.5";
        const char *const UsbDrive = "M8 3h8v6H8z M6 9h12v12H6z M10 5.5h1 M13 5.5h1";
        const char *const DiskImage = "M12 21a9 9 0 1 0 0-18a9 9 0 0 0 0 18z M12 14a2 2 0 1 0 0-4a2 2 0 0 0 0 4z";
        const char *const Warning = "M12 3l9 17H3z M12 10v4 M12 17h.01";
        const char *const Check = "M5 12l5 5 9-10";
        const char *const CheckCircle = "M12 21a9 9 0 1 0 0-18a9 9 0 0 0 0 18z M7.5 12.5l3 3 6-7";
        const char *const Image = "M4 5h16v14H4z M4 16l5-5 4 4 3-3 4 4 M15.5 9.5h.01";
        const char *const Video = "M3 6h13v12H3z M16 10l5-3v10l-5-3";
        const char *const Music = "M9 18V5l11-2v13 M9 18a3 3 0 1 1-6 0a3 3 0 0 1 6 0 M20 16a3 3 0 1 1-6 0a3 3 0 0 1 6 0";
        const char *const Document = "M6 3h8l4 4v14H6z M14 3v4h4 M9 12h6 M9 16h6";
        const char *const Text = "M6 3h8l4 4v14H6z M14 3v4h4 M9 10h6 M9 13h6 M9 16h4";
        const char *const Archive = "M4 4h16v4H4z M5 8v12h14V8 M10 12h4";
        const char *const Other = "M6 3h8l4 4v14H6z M14 3v4h4";
    }

    static wxString Hex(const wxColour &colour) {
        return colour.GetAsString(wxC2S_HTML_SYNTAX);
    }

    // a canvas of canvas.x by canvas.y with the 24x24 drawing scaled to iIconSize and centred
    static wxBitmapBundle FromSvgBody(const wxString &sBody, const wxSize &canvas, int iIconSize) {
        double dScale = 24.0 / iIconSize; // viewBox units per pixel
        double dWidth = canvas.x * dScale, dHeight = canvas.y * dScale;
        wxString sSvg = "<svg xmlns='http://www.w3.org/2000/svg' width='" + wxString::Format("%d", canvas.x)
                + "' height='" + wxString::Format("%d", canvas.y) + "' viewBox='"
                + wxString::FromCDouble((24 - dWidth) / 2) + " " + wxString::FromCDouble((24 - dHeight) / 2) + " "
                + wxString::FromCDouble(dWidth) + " " + wxString::FromCDouble(dHeight) + "'>" + sBody + "</svg>";
        return wxBitmapBundle::FromSVG(sSvg.utf8_str(), canvas);
    }

    wxBitmapBundle IconOnCanvas(const char *szPath, const wxColour &colour, const wxSize &canvas, int iIconSize, double dStrokeWidth) {
        return FromSvgBody("<path d='" + wxString(szPath) + "' fill='none' stroke='" + Hex(colour) + "' stroke-width='"
                + wxString::FromCDouble(dStrokeWidth) + "' stroke-linecap='round' stroke-linejoin='round'/>", canvas, iIconSize);
    }

    wxBitmapBundle Icon(const char *szPath, const wxColour &colour, int iSize, double dStrokeWidth) {
        return IconOnCanvas(szPath, colour, wxSize(iSize, iSize), iSize, dStrokeWidth);
    }

    wxBitmapBundle Dot(const wxColour &colour, const wxSize &canvas, int iDiameter) {
        // a circle of the full 24 units, scaled down to the diameter
        return FromSvgBody("<circle cx='12' cy='12' r='12' fill='" + Hex(colour) + "'/>", canvas, iDiameter);
    }

    wxBitmapBundle CheckBox(bool bChecked, const wxSize &canvas, int iBoxSize) {
        if (!bChecked)
            return FromSvgBody("<rect x='1' y='1' width='22' height='22' rx='4' fill='#FFFFFF' stroke='#8A8A8A' stroke-width='1.5'/>",
                canvas, iBoxSize);
        return FromSvgBody("<rect x='0.5' y='0.5' width='23' height='23' rx='4' fill='" + Hex(Accent) + "'/>"
                "<path d='M6 12.5l4 4 8-9' fill='none' stroke='#FFFFFF' stroke-width='2.6' stroke-linecap='round' stroke-linejoin='round'/>",
                canvas, iBoxSize);
    }

    const char *FileTypeIcon(int iFileType) {
        switch (iFileType) {
            case FILE_TYPE_IMAGE: return Icons::Image;
            case FILE_TYPE_VIDEO: return Icons::Video;
            case FILE_TYPE_MUSIC: return Icons::Music;
            case FILE_TYPE_DOCUMENT: return Icons::Document;
            case FILE_TYPE_TEXT: return Icons::Text;
            case FILE_TYPE_ARCHIVE: return Icons::Archive;
            default: return Icons::Other;
        }
    }

    void DrawRoundedBox(wxDC &dc, const wxRect &rect, double dRadius, const wxColour &fill, const wxColour &border, int iBorderWidth) {
        dc.SetBrush(fill.IsOk() ? wxBrush(fill) : *wxTRANSPARENT_BRUSH);
        dc.SetPen(border.IsOk() && iBorderWidth > 0 ? wxPen(border, iBorderWidth) : *wxTRANSPARENT_PEN);
        // the pen is centred on the outline - keep it inside the rectangle
        wxRect r(rect);
        if (border.IsOk() && iBorderWidth > 0)
            r.Deflate(iBorderWidth / 2);
        dc.DrawRoundedRectangle(r, dRadius);
    }
}
