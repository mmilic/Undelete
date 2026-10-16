/*
 * File:   MainFrame.h
 *
 * The app's window: pick a drive and its deleted files stream into the list while it is being scanned -
 * they can be ticked right away. Filters on the left, details and preview on the right, the scan's progress
 * and "Recover" at the bottom. Recover asks where to (CRecoverDialog) and shows the copying there.
 *
 * The scan runs in a thread (MyThreadClass.h): the window polls its progress and takes the files it found
 * with a timer. Meanwhile nothing else reads the drive - no previews, no content checks - and recovering
 * stops the scan first: the engines aren't thread-safe.
 */

#ifndef MAINFRAME_H
#define	MAINFRAME_H

#include <wx/wx.h>
#include <wx/simplebook.h>
#include <wx/srchctrl.h>
#include <wx/listctrl.h>
#include <vector>
#include "RecoveryDrive.h"
#include "DeletedFilesList.h"
#include "Widgets.h"

class CFileDetailsPanel;
class CScanThread;

class CMainFrame : public wxFrame {
public:
    // vImages: disk images given on the command line, offered beside the drives
    CMainFrame(const std::vector<wxString> &vImages);
    ~CMainFrame();

private:
    ST_DRIVE m_source; // the drive being scanned, or the last one
    bool m_bHasSource;
    FileInfoHash m_hash; // the scan's files - the list points into it
    CRecoveryDrive *m_pDrive; // the scanned drive's engine
    CScanThread *m_pScan;
    bool m_bScanStopped; // the last scan was stopped - the list holds only part of the drive
    wxString m_sLastDestination;
    wxTimer m_scanTimer; // progress and new files of the running scan
    wxTimer m_searchTimer; // the list is filtered a moment after typing stops

    std::vector<CDriveCard *> m_vCards;
    wxSearchCtrl *m_pSearch;
    wxRadioButton *m_apType[FILE_TYPE_COUNT];
    wxStaticText *m_apTypeCount[FILE_TYPE_COUNT];
    wxRadioButton *m_apShow[RECOVERABILITY_FILTER_COUNT];
    wxCheckBox *m_pValidOnly;
    CFlatButton *m_pCheckContents;
    wxSimplebook *m_pCenter;
    CDeletedFilesList *m_pList;
    CFileDetailsPanel *m_pDetails;
    CThinBar *m_pBar;
    wxStaticText *m_pStatus;
    CFlatButton *m_pStop;
    CFlatButton *m_pRecover;

    wxWindow *CreateDriveStrip(wxWindow *parent, const std::vector<ST_DRIVE> &vDrives);
    wxWindow *CreateFilters(wxWindow *parent);
    wxWindow *CreateCenter(wxWindow *parent);
    wxWindow *CreateBottomBar(wxWindow *parent);

    void StartScan(const ST_DRIVE &drive);
    void StopScan(); // wait for the scan thread and complete what it found
    void TakeNewFiles();
    CRecoveryDrive *DriveForReading() const { return m_pScan ? NULL : m_pDrive; }

    void UpdateCenter(); // which of: pick a drive, waiting for the first files, the list, nothing matches
    void UpdateCounts(); // the numbers beside the types
    void UpdateStatus(); // the bottom bar

    void OnDriveCard(wxCommandEvent &event);
    void OnScanTimer(wxTimerEvent &event);
    void OnThreadDone(wxThreadEvent &event);
    void OnStopScan(wxCommandEvent &event);
    void OnRecover(wxCommandEvent &event);
    void OnSearchText(wxCommandEvent &event);
    void OnSearchTimer(wxTimerEvent &event);
    void OnTypeFilter(wxCommandEvent &event);
    void OnShowFilter(wxCommandEvent &event);
    void OnValidOnly(wxCommandEvent &event);
    void OnCheckContents(wxCommandEvent &event);
    void OnClearFilters(wxCommandEvent &event);
    void OnChecked(wxCommandEvent &event);
    void OnFocused(wxListEvent &event);
    void OnClose(wxCloseEvent &event);
    bool CheckContents(const std::vector<t_FileInfo *> &vFiles, bool bShowSummary);
    void FiltersChanged();
};

#endif	/* MAINFRAME_H */
