/*
 * File:   Theme.h
 *
 * The look of the app in one place: colours, fonts and icons. The icons are the stroke icons of the
 * design (24x24 SVG paths), drawn by nanosvg at whatever size the screen's DPI needs - sharp at 100 %
 * and at 250 % alike.
 */

#ifndef THEME_H
#define	THEME_H

#include <wx/wx.h>
#include <wx/bmpbndl.h>

namespace Theme {
    // colours
    extern const wxColour Accent; // buttons, the current step, links
    extern const wxColour AccentHover;
    extern const wxColour WindowBg; // around the content
    extern const wxColour PanelBg; // the content
    extern const wxColour SubtleBg; // boxes inside the content (preview, progress)
    extern const wxColour FooterBg;
    extern const wxColour Border; // between areas
    extern const wxColour ControlBorder; // buttons, inputs
    extern const wxColour Text;
    extern const wxColour TextSecondary;
    extern const wxColour TextDisabled;
    extern const wxColour Good; // "Good", "Looks valid"
    extern const wxColour GoodDot;
    extern const wxColour Partial; // "Partly overwritten"
    extern const wxColour PartialDot;
    extern const wxColour Overwritten; // "Overwritten", "Start overwritten"
    extern const wxColour OverwrittenDot;
    extern const wxColour Damaged;
    extern const wxColour UsageHigh; // a drive more than 80 % full
    extern const wxColour WarningBg, WarningBorder, WarningText, WarningIcon;
    extern const wxColour InfoBg;
    extern const wxColour RowFocused; // the focused file in the list
    extern const wxColour RowChecked; // a file ticked for recovery

    // fonts - the design's sizes are in px at 96 DPI
    wxFont Body(); // 14 px
    wxFont Small(); // 13 px
    wxFont Tiny(); // 12 px
    wxFont Title(); // 20 px, semibold
    wxFont Pixels(double dPixels); // any other size
    wxFont Bold(const wxFont &font);

    // the design's stroke icons: an SVG path in a 24x24 box
    namespace Icons {
        extern const char *const App;
        extern const char *const HardDisk;
        extern const char *const UsbDrive;
        extern const char *const DiskImage;
        extern const char *const Warning;
        extern const char *const Check;
        extern const char *const CheckCircle;
        extern const char *const Image, *const Video, *const Music, *const Document, *const Text, *const Archive, *const Other;
    }

    // iSize x iSize at 100 % - the bundle makes it sharp at any DPI
    wxBitmapBundle Icon(const char *szPath, const wxColour &colour, int iSize, double dStrokeWidth = 1.6);
    // the icon centred on a wider/taller transparent canvas (list rows are as tall as their images)
    wxBitmapBundle IconOnCanvas(const char *szPath, const wxColour &colour, const wxSize &canvas, int iIconSize,
            double dStrokeWidth = 1.6);
    wxBitmapBundle Dot(const wxColour &colour, const wxSize &canvas, int iDiameter);
    wxBitmapBundle CheckBox(bool bChecked, const wxSize &canvas, int iBoxSize);

    // the icon of a FILE_TYPE_* (DeletedFilesList.h)
    const char *FileTypeIcon(int iFileType);

    // a rounded rectangle filled and/or outlined - the shape of buttons, cards and boxes
    void DrawRoundedBox(wxDC &dc, const wxRect &rect, double dRadius, const wxColour &fill, const wxColour &border,
            int iBorderWidth = 1);
}

#endif	/* THEME_H */
