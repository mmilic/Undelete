/*
 * File:   FileDetailsPanel.h
 *
 * The pane beside the file list: a preview of the focused file (read straight from the drive - nothing is
 * written to it) and what is known about it. Showing a file also checks its contents, one small read.
 */

#ifndef FILEDETAILSPANEL_H
#define	FILEDETAILSPANEL_H

#include <wx/wx.h>
#include <functional>
#include "RecoveryDrive.h"

class CPreviewBox;

class CFileDetailsPanel : public wxPanel {
public:
    CFileDetailsPanel(wxWindow *parent, int iWidth);

    // pDrive is NULL while the drive is busy (a scan or a recovery reads it) - then nothing is read
    void ShowFile(t_FileInfo *pFile, CRecoveryDrive *pDrive);
    t_FileInfo *GetFile() const { return m_pFile; }

    // called when showing a file checked its contents, so its row can show the result too
    std::function<void(t_FileInfo *)> OnContentChecked;

private:
    t_FileInfo *m_pFile;
    CRecoveryDrive *m_pDrive;
    int m_iWidth;
    CPreviewBox *m_pPreview;
    wxStaticText *m_pName;
    wxStaticText *m_pFolder, *m_pSize, *m_pModified, *m_pRecoverable, *m_pContents, *m_pStatus;
    wxStaticText *m_pStatusLabel;
    wxStaticText *m_pHint;
    wxTimer m_timer; // the preview is read a moment after the focus stops moving

    void UpdateTexts();
    void OnTimer(wxTimerEvent &event);
};

#endif	/* FILEDETAILSPANEL_H */
