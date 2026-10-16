/*
 * File:   MyThreadClass.h
 * Author: MilicM
 *
 * Created on October 8, 2012, 3:04 PM
 */

#ifndef MYTHREADCLASS_H
#define	MYTHREADCLASS_H

#endif	/* MYTHREADCLASS_H */

DECLARE_EVENT_TYPE(wxEVT_SELECT_DESTINATION_DRIVE_PAGE_PULSE_DIALOG, -1)
DECLARE_EVENT_TYPE(wxEVT_SELECT_DESTINATION_DRIVE_PAGE_STOP_DIALOG, -1)
DECLARE_EVENT_TYPE(wxEVT_SELECT_FILES_PAGE_PULSE_DIALOG, -1)
DECLARE_EVENT_TYPE(wxEVT_SELECT_FILES_PAGE_STOP_DIALOG, -1)


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
    void SetDestinationDirPtr(wxString *ptr);
    void SetSourceDrive(wxString *ptr);
    void SetIptrNumberOfCopiedFiles(int* iptrNumberOfCopiedFiles);
    int* GetIptrNumberOfCopiedFiles() const;

    void ThrowSelectDestinationDrivePulseEvent(wxString wxstrMessage);
    void ThrowSelectDestinationDriveStopEvent();
    void ThrowSelectFilesDialogPulseEvent(wxString wxstrMessage, int iCount);
    void ThrowSelectFilesDialogStopEvent();


public:
    CNTFSDrive *m_pUsedNTFSDrive;
    FileInfoHash *m_finfohash;
    wxString *m_wxpDestinationDir;
    wxString *m_wxpSourceDrive;
    wxWindow *m_parent; //treba mi da bi ovaj thread mogao da baca evente
    int *m_iptrNumberOfCopiedFiles;
    volatile bool m_CancelProgresControl; //when set to true - the Thread processing will stop (set by the GUI thread)
    int m_iResult; // outcome of the thread's work, read by the GUI thread after Wait()

    void Init();
    void SetCancelProgresControl(bool CancelProgresControl);
    bool IsCancelProgresControl() const;

protected:
    void *ScanVolume(); // reads the volume and fills m_finfohash

private:

};


class MyThreadLoadFilesFromDiskNTFS : public MyThreadClass {
public:
    MyThreadLoadFilesFromDiskNTFS();
    MyThreadLoadFilesFromDiskNTFS(wxWindow *);
    void *Entry();
};

// "photo.jpg", 2 -> "photo (2).jpg" - the name for a recovered file when the original name is taken
wxString NumberedFileName(const wxString &sName, int iNumber);

class MyThreadCopyFiles : public MyThreadClass {
public:
    MyThreadCopyFiles();
    MyThreadCopyFiles(wxWindow *);
    void *Entry();
    void SetFilesToCopy(const std::vector<t_FileInfo *> &vFiles);

    std::vector<t_FileInfo *> m_vFilesToCopy;

    volatile LONG m_lKBWritten; // progress: KB written so far, all files together (read by the GUI thread)
    std::vector<wxString> m_vFailures; // "name - reason" for each file that could not be recovered (read after Wait())
};
