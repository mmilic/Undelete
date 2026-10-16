// AppLog.cpp: the UTF-8 log file in the destination folder

#include <windows.h>
#include <wx/wx.h>
#include <wx/filename.h>
#include "AppLog.h"

#define MAX_PENDING_LINES 20000

static CAppLog *s_pLog = NULL;

CAppLog::CAppLog() {
    m_fp = NULL;
}

CAppLog::~CAppLog() {
    if (m_fp)
        fclose(m_fp);
    if (s_pLog == this)
        s_pLog = NULL;
}

CAppLog *CAppLog::Install() {
    s_pLog = new CAppLog;
    delete wxLog::SetActiveTarget(s_pLog);
    wxLog::SetTimestamp("%H:%M:%S");
    return s_pLog;
}

CAppLog *CAppLog::Get() {
    return s_pLog;
}

bool CAppLog::OpenFile(const wxString &sFolder) {
    SYSTEMTIME time;
    GetLocalTime(&time);
    wxFileName name;
    name.AssignDir(sFolder);
    name.SetFullName(wxString::Format("UndeleteLogFile_%d.%d.%d@%dh%dm%ds.log", time.wYear, time.wMonth, time.wDay,
            time.wHour, time.wMinute, time.wSecond));

    // _wfopen - the folder's name may contain characters outside the ANSI code page
    FILE *fp = _wfopen(name.GetFullPath().wc_str(), L"wb");
    if (!fp)
        return false;
    if (m_fp)
        fclose(m_fp);
    m_fp = fp;
    fputs("\xEF\xBB\xBF", m_fp); // UTF-8 BOM, so Notepad shows the file names correctly
    for (size_t i = 0; i < m_asPending.size(); i++) {
        fputs(m_asPending[i].utf8_str(), m_fp);
        fputs("\n", m_fp);
    }
    m_asPending.clear();
    fflush(m_fp);
    return true;
}

// UTF-8: wxLogStderr would convert to the ANSI code page and silently drop every line with a file name
// it can't convert (Cyrillic, most accented letters...)

void CAppLog::DoLogText(const wxString &sMessage) {
    if (!m_fp) {
        if (m_asPending.size() < MAX_PENDING_LINES)
            m_asPending.push_back(sMessage);
        return;
    }
    fputs(sMessage.utf8_str(), m_fp);
    fputs("\n", m_fp);
    fflush(m_fp);
}
