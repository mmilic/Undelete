/*
 * File:   RecoverDialog.h
 *
 * "Recover N files": where to (never the drive they come from), then the copying, then what came of it.
 * The dialog runs the copy thread itself and polls its progress; the scan has stopped before it opens,
 * so the thread has the drive to itself.
 */

#ifndef RECOVERDIALOG_H
#define	RECOVERDIALOG_H

#include <wx/wx.h>
#include <wx/wrapsizer.h>
#include <vector>
#include "RecoveryDrive.h"
#include "Widgets.h"

class CCopyThread;

class CRecoverDialog : public wxDialog {
public:
    CRecoverDialog(wxWindow *parent, CRecoveryDrive *pDrive, const ST_DRIVE &source, const std::vector<t_FileInfo *> &vFiles,
            const wxString &sLastDestination);
    ~CRecoverDialog();

    // the folder the files were recovered to - empty if nothing was started
    wxString GetDestination() const { return m_sUsedDestination; }

private:
    CRecoveryDrive *m_pDrive;
    ST_DRIVE m_source;
    std::vector<t_FileInfo *> m_vFiles;
    LONGLONG m_llBytes;
    CCopyThread *m_pCopy;
    wxString m_sUsedDestination;
    wxTimer m_timer;

    wxStaticText *m_pTitle;
    // choosing the folder
    wxPanel *m_pChoose;
    wxTextCtrl *m_pPath;
    wxWrapSizer *m_pChips;
    CBanner *m_pWarning;
    CFlatButton *m_pStart;
    // copying
    wxPanel *m_pCopying;
    wxStaticText *m_pCopyText;
    CThinBar *m_pBar;
    // done
    wxPanel *m_pDone;
    wxStaticText *m_pResult;
    wxStaticText *m_pFolder;
    wxStaticText *m_pNote;
    wxStaticText *m_pFailuresLabel;
    wxTextCtrl *m_pFailures;

    wxPanel *CreateChoose();
    wxPanel *CreateCopying();
    wxPanel *CreateDone();
    void ShowPhase(wxPanel *pPhase);
    wxString GetPath() const;
    bool IsValid() const;
    void CheckPath();

    void OnStart(wxCommandEvent &event);
    void OnTimer(wxTimerEvent &event);
    void OnThreadDone(wxThreadEvent &event);
    void OnCancel(wxCommandEvent &event);
    void OnClose(wxCloseEvent &event);
};

#endif	/* RECOVERDIALOG_H */
