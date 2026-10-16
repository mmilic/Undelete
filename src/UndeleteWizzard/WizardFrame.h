/*
 * File:   WizardFrame.h
 *
 * The app's window: four steps - source drive, destination, select files, done - with the step list on
 * the left, the current step's page on the right and Back / Next / Cancel at the bottom. The window holds
 * what the steps share (the chosen drive, the destination, the scan results, the drive's engine); the
 * pages show it and tell the window when something changed (UpdateFooter).
 *
 * The scan and the recovery run in threads (MyThreadClass.h). Meanwhile the window stays usable: the
 * progress is polled by a timer, Cancel stops the thread, and the drive isn't read by anything else
 * (no previews, no content checks) until the thread is done.
 */

#ifndef WIZARDFRAME_H
#define	WIZARDFRAME_H

#include <wx/wx.h>
#include <wx/simplebook.h>
#include <wx/srchctrl.h>
#include <wx/wrapsizer.h>
#include <wx/listctrl.h>
#include <vector>
#include "RecoveryDrive.h"
#include "Widgets.h"

class CWizardFrame;
class CDeletedFilesList;
class CFileDetailsPanel;
class CScanThread;
class CCopyThread;

enum {
    STEP_SOURCE = 0,
    STEP_DESTINATION,
    STEP_FILES,
    STEP_DONE,
    STEP_COUNT
};

// step 1: the drive cards

class CSourcePage : public wxScrolledWindow {
public:
    CSourcePage(wxWindow *parent, CWizardFrame *pFrame, const std::vector<ST_DRIVE> &vDrives);
    void ShowSelection(const wxString &sSource);

private:
    CWizardFrame *m_pFrame;
    std::vector<CDriveCard *> m_vCards;
    void OnCard(wxCommandEvent &event);
};

// step 2: where the recovered files go

class CDestinationPage : public wxPanel {
public:
    CDestinationPage(wxWindow *parent, CWizardFrame *pFrame);
    void OnShow(); // the source may have changed - new suggestions
    wxString GetDestination() const;
    bool IsValid() const; // a folder, not on the source drive

private:
    CWizardFrame *m_pFrame;
    wxTextCtrl *m_pPath;
    wxWrapSizer *m_pChips;
    wxStaticText *m_pChipsLabel;
    CBanner *m_pWarning;
    CBanner *m_pOk;
    wxString m_sSuggested; // what this page filled in itself - replaced when the source changes
    void CheckDestination();
    void OnBrowse(wxCommandEvent &event);
    void OnChip(wxCommandEvent &event);
};

// step 3: scanning, then the list of deleted files - and the progress of the recovery

class CProgressBox;

class CFilesPage : public wxPanel {
public:
    CFilesPage(wxWindow *parent, CWizardFrame *pFrame);

    void ShowProgress(const wxString &sTitle, const wxString &sNote);
    void SetProgress(int iPercent, const wxString &sNote); // -1: indeterminate
    void ShowList(FileInfoHash *pHash); // a new scan's files - nothing ticked
    void ShowListAgain(); // after a recovery: the same files, the same ticks
    void ClearList();

    CDeletedFilesList *GetList() const { return m_pList; }
    void SetDriveForPreview(CRecoveryDrive *pDrive); // NULL while the drive is busy
    void UpdateSummary();

private:
    CWizardFrame *m_pFrame;
    wxPanel *m_pProgressView;
    CProgressBox *m_pProgress;
    wxPanel *m_pListView;
    wxSearchCtrl *m_pSearch;
    CSegmented *m_pShow;
    wxChoice *m_pType;
    CFlatButton *m_pCheckContents;
    CDeletedFilesList *m_pList;
    wxStaticText *m_pSummary;
    CFileDetailsPanel *m_pDetails;
    CRecoveryDrive *m_pDrive;
    wxTimer m_searchTimer; // the list is filtered a moment after typing stops

    void OnSearchText(wxCommandEvent &event);
    void OnSearchTimer(wxTimerEvent &event);
    void OnShowFilter(wxCommandEvent &event);
    void OnTypeFilter(wxCommandEvent &event);
    void OnCheckContents(wxCommandEvent &event);
    void OnChecked(wxCommandEvent &event);
    void OnFocused(wxListEvent &event);
};

// step 4: what was recovered

class CDonePage : public wxPanel {
public:
    CDonePage(wxWindow *parent, CWizardFrame *pFrame);
    void ShowResult(int iRecovered, int iSelected, int iPartly, int iOverwritten, bool bCancelled,
            const std::vector<wxString> &vFailures, const wxString &sDestination);

private:
    CWizardFrame *m_pFrame;
    wxStaticText *m_pTitle;
    wxStaticText *m_pFolder; // a line of its own - a long path in the title would wrap out of sight
    wxStaticText *m_pNote;
    wxStaticText *m_pFailuresLabel;
    wxTextCtrl *m_pFailures;
    wxString m_sDestination;
    void OnOpenFolder(wxCommandEvent &event);
    void OnMore(wxCommandEvent &event);
};

class CWizardFrame : public wxFrame {
public:
    // vImages: disk images given on the command line, offered beside the drives
    CWizardFrame(const std::vector<wxString> &vImages);
    ~CWizardFrame();

    const ST_DRIVE *GetSource() const { return m_bHasSource ? &m_source : NULL; }
    void SetSource(const ST_DRIVE &drive);
    CRecoveryDrive *GetDrive() const { return m_pDrive; }
    bool IsBusy() const { return m_pScan || m_pCopy; } // a thread reads the drive

    void UpdateFooter(); // a page's state changed - the buttons and the hint follow it
    void GoToStep(int iStep);

private:
    int m_iStep;
    ST_DRIVE m_source;
    bool m_bHasSource;
    wxString m_sDestination; // the folder the last scan/recovery used
    bool m_bScanStopped; // the last scan was stopped - the list holds only part of the drive

    FileInfoHash m_hash; // the scan's files - the list points into it
    CRecoveryDrive *m_pDrive; // the scanned drive's engine
    CScanThread *m_pScan;
    CCopyThread *m_pCopy;
    LONGLONG m_llCopyBytes; // all files of the recovery together
    wxTimer m_progressTimer;

    CStepList *m_pSteps;
    wxStaticText *m_pHeading;
    wxStaticText *m_pSubheading;
    wxSimplebook *m_pBook;
    CSourcePage *m_pSourcePage;
    CDestinationPage *m_pDestinationPage;
    CFilesPage *m_pFilesPage;
    CDonePage *m_pDonePage;
    wxStaticText *m_pHint;
    CFlatButton *m_pBack;
    CFlatButton *m_pNext;
    CFlatButton *m_pCancel;

    void SetHeading(const wxString &sHeading, const wxString &sSubheading);
    void WrapSubheading();
    wxString m_sSubheading;
    void StartScan();
    void StartRecovery();
    void OnThreadDone(wxThreadEvent &event);
    void OnScanDone();
    void OnCopyDone();
    void OnProgressTimer(wxTimerEvent &event);
    void OnBack(wxCommandEvent &event);
    void OnNext(wxCommandEvent &event);
    void OnCancel(wxCommandEvent &event);
    void OnClose(wxCloseEvent &event);
    void StopThreads();
};

#endif	/* WIZARDFRAME_H */
