/*
 * File:   AppLog.h
 *
 * The log file, UTF-8, in the destination folder - so a user can send it along with a question. Until the
 * destination is chosen the lines are kept in memory, and they go to the file first once it is open.
 * Nothing is ever shown as a message box: errors reach the user through the window, worded for them.
 */

#ifndef APPLOG_H
#define	APPLOG_H

#include <wx/log.h>
#include <wx/arrstr.h>
#include <stdio.h>

class CAppLog : public wxLog {
public:
    CAppLog();
    ~CAppLog();

    // the app's log from now on
    static CAppLog *Install();
    static CAppLog *Get();

    // UndeleteLogFile_<date and time>.log in the folder; again for another folder starts a new file
    bool OpenFile(const wxString &sFolder);

protected:
    void DoLogText(const wxString &sMessage) override;

private:
    FILE *m_fp;
    wxArrayString m_asPending; // lines logged before there was a file
};

#endif	/* APPLOG_H */
