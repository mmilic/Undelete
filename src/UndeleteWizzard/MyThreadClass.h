/* 
 * File:   MyThreadClass.h
 * Author: MilicM
 *
 * Created on October 8, 2012, 3:04 PM
 */

#ifndef MYTHREADCLASS_H
#define	MYTHREADCLASS_H

#endif	/* MYTHREADCLASS_H */

DECLARE_EVENT_TYPE(wxEVT_SHOW_NOTIFICATION, -1)
DECLARE_EVENT_TYPE(wxEVT_ENABLE_GUI_FIELDS, -1)
DECLARE_EVENT_TYPE(wxEVT_SELECT_DESTINATION_DRIVE_PAGE_PULSE_DIALOG, -1)
DECLARE_EVENT_TYPE(wxEVT_SELECT_DESTINATION_DRIVE_PAGE_STOP_DIALOG, -1)
DECLARE_EVENT_TYPE(wxEVT_SELECT_FILES_PAGE_PULSE_DIALOG, -1)
DECLARE_EVENT_TYPE(wxEVT_SELECT_FILES_PAGE_STOP_DIALOG, -1)
DECLARE_EVENT_TYPE(wxEVT_SELECT_FILES_SEARCH_ACTION, -1)


class MyThreadClass : public wxThread {
public:
    MyThreadClass();
    //    MyThreadClass(wxFrame *parent);
    MyThreadClass(wxWindow *parent);

    virtual ~MyThreadClass();

    // thread execution starts here
    virtual void *Entry();

    void SetNTFSDrivePtr(CNTFSDrive *ptr);
    void SetFinfoHashPtr(FileInfoHash *ptr);
    void SetListControlPtr(wxListCtrl *ptr);
    void SetDestinationDirPtr(wxString *ptr);
    void SetSourceDrive(wxString *ptr);
    void SetMutexLockPtr(wxMutex *ptr);
    void SetLogHndl(HANDLE hFile = NULL);
    void SetIptrNumberOfCopiedFiles(int* iptrNumberOfCopiedFiles);
    int* GetIptrNumberOfCopiedFiles() const;

    void ThrowEnableGuiEvent();
    void ThrowInfoEvent(wxString wxstrMessage);
    void ThrowSelectDestinationDrivePulseEvent(wxString wxstrMessage);
    void ThrowSelectDestinationDriveStopEvent();
    void ThrowSelectFilesDialogPulseEvent(wxString wxstrMessage);
    void ThrowSelectFilesDialogPulseEvent(wxString wxstrMessage, int iCount);
    void ThrowSelectFilesDialogStopEvent();
    void ThrowSearchActionEvent();


public:
    unsigned m_count;
    CNTFSDrive *m_pUsedNTFSDrive;
    FileInfoHash *m_finfohash;
    wxListCtrl *m_ListControl;
    wxString *m_wxpDestinationDir;
    wxString *m_wxpSourceDrive;
    wxMutex *m_LockPtr;
    wxWindow *m_parent; //treba mi da bi ovaj thread mogao da baca evente
    int *m_iptrNumberOfCopiedFiles;
    HANDLE hFileLog;
    bool m_CancelProgresControl; //when set to true - the Thread processing will stop

    void Init();
    void SetCancelProgresControl(bool CancelProgresControl);
    bool IsCancelProgresControl() const;

private:

};


class MyThreadLoadFilesFromDiskFAT : public MyThreadClass {
public:
    MyThreadLoadFilesFromDiskFAT();
    MyThreadLoadFilesFromDiskFAT(wxWindow *);
    void FillListControlFromHashMap();
    void *Entry();
};

class MyThreadLoadFilesFromDiskNTFS : public MyThreadClass {
public:
    MyThreadLoadFilesFromDiskNTFS();
    MyThreadLoadFilesFromDiskNTFS(wxWindow *);
    void FillListControlFromHashMap();
    void *Entry();
};

class MyThreadCopyFiles : public MyThreadClass {
public:
    MyThreadCopyFiles();
    MyThreadCopyFiles(wxWindow *);
    void *Entry();
    void SetDestTextBox(wxTextCtrl *);
    void SetMwPtr_strSelectedDir(wxString* mwPtr_strSelectedDir);
    wxString* GetMwPtr_strSelectedDir() const;

    wxTextCtrl * m_DestTextBox; // ovo ce moci da se brise verovatno
    wxString *mwPtr_strSelectedDir;
};

class MyThreadSearchAction : public MyThreadClass {
public:
    MyThreadSearchAction();
    MyThreadSearchAction(wxWindow *);
    void FillListControlFromHashMap();
    void *Entry();
};