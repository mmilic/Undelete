#include <windows.h>
#include <commctrl.h>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <algorithm>
#include <set>

#include "DeletedFilesList.h"
#include "FileContents.h"

// order of the "Recoverable" column: most promising first

static int RecoverabilityRank(const t_FileInfo *pFile) {
    switch (pFile->iRecoverability) {
        case RECOVERABILITY_GOOD: return 0;
        case RECOVERABILITY_PARTIAL: return 1;
        case RECOVERABILITY_UNKNOWN: return 2;
        default: return 3; // overwritten
    }
}

// order of the "Contents" column: valid first, not checked last

static int ContentRank(const t_FileInfo *pFile) {
    switch (pFile->iContent) {
        case CONTENT_OK: return 0;
        case CONTENT_NO_SIGNATURE: return 1;
        case CONTENT_UNREADABLE: return 2;
        case CONTENT_DAMAGED: return 3;
        case CONTENT_ZEROS: return 4;
        case CONTENT_START_OVERWRITTEN: return 5;
        default: return 6; // not checked
    }
}

static int CompareLongLong(LONGLONG a, LONGLONG b) {
    return (a < b) ? -1 : (a > b ? 1 : 0);
}

struct DeletedFilesOrder {
    int iColumn;
    bool bAscending;

    bool operator()(const t_FileInfo *a, const t_FileInfo *b) const {
        int r = 0;
        switch (iColumn) {
            case DELETED_FILES_COLUMN_FOLDER:
                if (a->sFolder.c_str() != b->sFolder.c_str()) // files of one folder share the string
                    r = _wcsicmp(a->sFolder.c_str(), b->sFolder.c_str());
                break;
            case DELETED_FILES_COLUMN_SIZE:
                r = CompareLongLong(a->n64RealSize, b->n64RealSize);
                break;
            case DELETED_FILES_COLUMN_MODIFIED:
                r = CompareLongLong(a->n64Modify, b->n64Modify);
                break;
            case DELETED_FILES_COLUMN_RECOVERABILITY:
                r = RecoverabilityRank(a) - RecoverabilityRank(b);
                if (!r) // more intact first
                    r = b->iPercentIntact - a->iPercentIntact;
                break;
            case DELETED_FILES_COLUMN_CONTENTS:
                r = ContentRank(a) - ContentRank(b);
                if (!r)
                    r = RecoverabilityRank(a) - RecoverabilityRank(b);
                break;
            case DELETED_FILES_COLUMN_STATUS:
                r = (int) a->bInRecycleBin - (int) b->bInRecycleBin; // group by status without building its text
                if (!r)
                    r = (a->dwAttributes < b->dwAttributes) ? -1 : (a->dwAttributes > b->dwAttributes ? 1 : 0);
                break;
        }
        if (!r) // ties (and the name column) - by name, so the order is stable and predictable
            r = _wcsicmp(a->sFilename.c_str(), b->sFilename.c_str());
        if (!r)
            r = a->iRecordId - b->iRecordId;
        return bAscending ? (r < 0) : (r > 0);
    }
};

wxString FormatFileSize(LONGLONG llSize) {
    if (llSize < 1024)
        return wxString::Format("%d B", (int) llSize);
    if (llSize < 1024 * 1024)
        return wxString::Format("%.1f KB", llSize / 1024.0);
    if (llSize < 1024LL * 1024 * 1024)
        return wxString::Format("%.1f MB", llSize / (1024.0 * 1024));
    return wxString::Format("%.2f GB", llSize / (1024.0 * 1024 * 1024));
}

struct FileTypeExtension {
    const wchar_t *szExtension;
    int iType;
};

static const FileTypeExtension s_aFileTypes[] = {
    {L"jpg", FILE_TYPE_IMAGE}, {L"jpeg", FILE_TYPE_IMAGE}, {L"jpe", FILE_TYPE_IMAGE}, {L"gif", FILE_TYPE_IMAGE},
    {L"png", FILE_TYPE_IMAGE}, {L"bmp", FILE_TYPE_IMAGE}, {L"tif", FILE_TYPE_IMAGE}, {L"tiff", FILE_TYPE_IMAGE},
    {L"ico", FILE_TYPE_IMAGE}, {L"heic", FILE_TYPE_IMAGE}, {L"heif", FILE_TYPE_IMAGE}, {L"webp", FILE_TYPE_IMAGE},
    {L"psd", FILE_TYPE_IMAGE}, {L"svg", FILE_TYPE_IMAGE}, {L"raw", FILE_TYPE_IMAGE}, {L"cr2", FILE_TYPE_IMAGE},
    {L"nef", FILE_TYPE_IMAGE}, {L"arw", FILE_TYPE_IMAGE}, {L"dng", FILE_TYPE_IMAGE},

    {L"avi", FILE_TYPE_VIDEO}, {L"mov", FILE_TYPE_VIDEO}, {L"wmv", FILE_TYPE_VIDEO}, {L"mp4", FILE_TYPE_VIDEO},
    {L"mkv", FILE_TYPE_VIDEO}, {L"m4v", FILE_TYPE_VIDEO}, {L"mpg", FILE_TYPE_VIDEO}, {L"mpeg", FILE_TYPE_VIDEO},
    {L"3gp", FILE_TYPE_VIDEO}, {L"webm", FILE_TYPE_VIDEO}, {L"flv", FILE_TYPE_VIDEO}, {L"vob", FILE_TYPE_VIDEO},
    {L"mts", FILE_TYPE_VIDEO}, {L"m2ts", FILE_TYPE_VIDEO},

    {L"mp3", FILE_TYPE_MUSIC}, {L"wma", FILE_TYPE_MUSIC}, {L"wav", FILE_TYPE_MUSIC}, {L"flac", FILE_TYPE_MUSIC},
    {L"m4a", FILE_TYPE_MUSIC}, {L"ogg", FILE_TYPE_MUSIC}, {L"aac", FILE_TYPE_MUSIC}, {L"aif", FILE_TYPE_MUSIC},
    {L"aiff", FILE_TYPE_MUSIC}, {L"opus", FILE_TYPE_MUSIC}, {L"mid", FILE_TYPE_MUSIC}, {L"midi", FILE_TYPE_MUSIC},

    {L"pdf", FILE_TYPE_DOCUMENT}, {L"doc", FILE_TYPE_DOCUMENT}, {L"docx", FILE_TYPE_DOCUMENT}, {L"xls", FILE_TYPE_DOCUMENT},
    {L"xlsx", FILE_TYPE_DOCUMENT}, {L"ppt", FILE_TYPE_DOCUMENT}, {L"pptx", FILE_TYPE_DOCUMENT}, {L"odt", FILE_TYPE_DOCUMENT},
    {L"ods", FILE_TYPE_DOCUMENT}, {L"odp", FILE_TYPE_DOCUMENT}, {L"rtf", FILE_TYPE_DOCUMENT}, {L"epub", FILE_TYPE_DOCUMENT},
    {L"pages", FILE_TYPE_DOCUMENT}, {L"numbers", FILE_TYPE_DOCUMENT}, {L"key", FILE_TYPE_DOCUMENT},

    {L"txt", FILE_TYPE_TEXT}, {L"csv", FILE_TYPE_TEXT}, {L"log", FILE_TYPE_TEXT}, {L"md", FILE_TYPE_TEXT},
    {L"xml", FILE_TYPE_TEXT}, {L"json", FILE_TYPE_TEXT}, {L"htm", FILE_TYPE_TEXT}, {L"html", FILE_TYPE_TEXT},
    {L"ini", FILE_TYPE_TEXT},

    {L"zip", FILE_TYPE_ARCHIVE}, {L"rar", FILE_TYPE_ARCHIVE}, {L"7z", FILE_TYPE_ARCHIVE}, {L"gz", FILE_TYPE_ARCHIVE},
    {L"tgz", FILE_TYPE_ARCHIVE}, {L"tar", FILE_TYPE_ARCHIVE}, {L"bz2", FILE_TYPE_ARCHIVE}, {L"xz", FILE_TYPE_ARCHIVE},
    {L"cab", FILE_TYPE_ARCHIVE}, {L"iso", FILE_TYPE_ARCHIVE},
};

int FileTypeOf(const wchar_t *szFilename) {
    const wchar_t *pDot = wcsrchr(szFilename, L'.');
    if (!pDot || pDot == szFilename || !pDot[1]) // no extension (".profile" is a name, not an extension)
        return FILE_TYPE_OTHER;
    for (size_t i = 0; i < sizeof (s_aFileTypes) / sizeof (s_aFileTypes[0]); i++)
        if (!_wcsicmp(pDot + 1, s_aFileTypes[i].szExtension))
            return s_aFileTypes[i].iType;
    return FILE_TYPE_OTHER;
}

const char *FileTypeFilterLabel(int iType) {
    switch (iType) {
        case FILE_TYPE_ANY: return "All types";
        case FILE_TYPE_IMAGE: return "Images";
        case FILE_TYPE_VIDEO: return "Videos";
        case FILE_TYPE_MUSIC: return "Music";
        case FILE_TYPE_DOCUMENT: return "Documents (PDF, Office)";
        case FILE_TYPE_TEXT: return "Text";
        case FILE_TYPE_ARCHIVE: return "Archives";
        default: return "Other";
    }
}

const char *RecoverabilityFilterLabel(int iFilter) {
    switch (iFilter) {
        case RECOVERABILITY_FILTER_GOOD: return "Good only";
        case RECOVERABILITY_FILTER_NOT_OVERWRITTEN: return "Hide overwritten";
        case RECOVERABILITY_FILTER_CONTENT_OK: return "Contents look valid";
        default: return "All files";
    }
}

wxString FormatFileTime(LONGLONG llFileTime) {
    if (llFileTime <= 0)
        return wxEmptyString;
    FILETIME ftUtc, ftLocal;
    SYSTEMTIME st;
    ftUtc.dwLowDateTime = (DWORD) llFileTime;
    ftUtc.dwHighDateTime = (DWORD) (llFileTime >> 32);
    if (!FileTimeToLocalFileTime(&ftUtc, &ftLocal) || !FileTimeToSystemTime(&ftLocal, &st))
        return wxEmptyString;
    return wxString::Format("%04d-%02d-%02d %02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
}

CDeletedFilesList::CDeletedFilesList(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
: wxListCtrl(parent, id, pos, size, wxLC_REPORT | wxLC_VIRTUAL | wxBORDER_THEME | wxLC_HRULES | wxLC_VRULES) {
    m_iSortColumn = DELETED_FILES_COLUMN_RECOVERABILITY; // the most useful order: what can still be recovered first
    m_bSortAscending = true;
    m_iRecoverabilityFilter = RECOVERABILITY_FILTER_ALL;
    m_iTypeFilter = FILE_TYPE_ANY;

    InsertColumn(DELETED_FILES_COLUMN_NAME, wxT("File Name"), wxLIST_FORMAT_LEFT, 220);
    InsertColumn(DELETED_FILES_COLUMN_FOLDER, wxT("Folder"), wxLIST_FORMAT_LEFT, 200);
    InsertColumn(DELETED_FILES_COLUMN_SIZE, wxT("File Size"), wxLIST_FORMAT_RIGHT, 75);
    InsertColumn(DELETED_FILES_COLUMN_MODIFIED, wxT("Modified"), wxLIST_FORMAT_LEFT, 110);
    InsertColumn(DELETED_FILES_COLUMN_RECOVERABILITY, wxT("Recoverable"), wxLIST_FORMAT_LEFT, 150);
    InsertColumn(DELETED_FILES_COLUMN_CONTENTS, wxT("Contents"), wxLIST_FORMAT_LEFT, 110);
    InsertColumn(DELETED_FILES_COLUMN_STATUS, wxT("File Status"), wxLIST_FORMAT_LEFT, 130);

    m_attrPartial.SetTextColour(wxColour(170, 90, 0)); // dark orange
    m_attrOverwritten.SetTextColour(wxColour(150, 150, 150)); // grey

    SetItemCount(0);

    Connect(id, wxEVT_COMMAND_LIST_KEY_DOWN, wxListEventHandler(CDeletedFilesList::OnListKeyDown));
}

// Ctrl+A - the Windows list view doesn't do it by itself

void CDeletedFilesList::OnListKeyDown(wxListEvent &event) {
    if ((event.GetKeyCode() == 'A' || event.GetKeyCode() == 'a') && wxGetKeyState(WXK_CONTROL))
        SelectAll(true);
    event.Skip();
}

void CDeletedFilesList::SetFiles(FileInfoHash *finfohash) {
    m_vAll.clear();
    m_vAll.reserve(finfohash->size());
    for (FileInfoHash::iterator it = finfohash->begin(); it != finfohash->end(); ++it)
        m_vAll.push_back(it->second);

    SelectAll(false);
    Rebuild();
}

void CDeletedFilesList::Clear() {
    SelectAll(false);
    m_vAll.clear();
    m_vVisible.clear();
    SetItemCount(0);
    Refresh();
}

void CDeletedFilesList::SetNameFilter(const wxString &sFilter) {
    m_sFilterUpper = sFilter.Upper();
    Rebuild();
}

void CDeletedFilesList::SetRecoverabilityFilter(int iFilter) {
    m_iRecoverabilityFilter = iFilter;
    Rebuild();
}

void CDeletedFilesList::SetTypeFilter(int iType) {
    m_iTypeFilter = iType;
    Rebuild();
}

bool CDeletedFilesList::PassesFilters(const t_FileInfo *pFile) const {
    if (m_iRecoverabilityFilter == RECOVERABILITY_FILTER_GOOD && pFile->iRecoverability != RECOVERABILITY_GOOD)
        return false;
    if (m_iRecoverabilityFilter == RECOVERABILITY_FILTER_NOT_OVERWRITTEN && pFile->iRecoverability == RECOVERABILITY_OVERWRITTEN)
        return false;
    if (m_iRecoverabilityFilter == RECOVERABILITY_FILTER_CONTENT_OK && pFile->iContent != CONTENT_OK)
        return false;
    if (m_iTypeFilter != FILE_TYPE_ANY && FileTypeOf(pFile->sFilename.c_str()) != m_iTypeFilter)
        return false;
    if (!m_sFilterUpper.IsEmpty()) { // the whole path, so "Pictures\IMG" or a folder name finds files too
        wxString sPath(pFile->sFolder.c_str());
        if (!sPath.EndsWith("\\"))
            sPath += "\\";
        sPath += pFile->sFilename.c_str();
        if (!sPath.Upper().Contains(m_sFilterUpper))
            return false;
    }
    return true;
}

void CDeletedFilesList::SortByColumn(int iColumn) {
    if (iColumn < 0 || iColumn >= DELETED_FILES_COLUMN_COUNT)
        return;
    if (iColumn == m_iSortColumn)
        m_bSortAscending = !m_bSortAscending;
    else {
        m_iSortColumn = iColumn;
        m_bSortAscending = true;
    }
    Rebuild();
}

t_FileInfo *CDeletedFilesList::GetFile(long lItem) const {
    if (lItem < 0 || lItem >= (long) m_vVisible.size())
        return NULL;
    return m_vVisible[lItem];
}

void CDeletedFilesList::GetSelectedFiles(std::vector<t_FileInfo *> &vSelected) const {
    vSelected.clear();
    long lItem = -1;
    while ((lItem = GetNextItem(lItem, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1)
        if (lItem < (long) m_vVisible.size())
            vSelected.push_back(m_vVisible[lItem]);
}

void CDeletedFilesList::SelectAll(bool bSelect) {
    // index -1 changes every item in one message - item by item is far too slow with 100 000+ files
    LVITEM lvi;
    memset(&lvi, 0, sizeof (lvi));
    lvi.stateMask = LVIS_SELECTED;
    lvi.state = bSelect ? LVIS_SELECTED : 0;
    ::SendMessage((HWND) GetHWND(), LVM_SETITEMSTATE, (WPARAM) - 1, (LPARAM) & lvi);
}

void CDeletedFilesList::SelectItem(long lItem, bool bSelect) {
    SetItemState(lItem, bSelect ? wxLIST_STATE_SELECTED : 0, wxLIST_STATE_SELECTED);
}

void CDeletedFilesList::Rebuild() {
    // the selection belongs to row numbers - remember it by file, so filtering/sorting keeps it
    std::vector<t_FileInfo *> vSelected;
    GetSelectedFiles(vSelected);
    std::set<t_FileInfo *> setSelected(vSelected.begin(), vSelected.end());
    SelectAll(false);

    m_vVisible.clear();
    if (m_sFilterUpper.IsEmpty() && m_iRecoverabilityFilter == RECOVERABILITY_FILTER_ALL && m_iTypeFilter == FILE_TYPE_ANY)
        m_vVisible = m_vAll;
    else
        for (size_t i = 0; i < m_vAll.size(); i++)
            if (PassesFilters(m_vAll[i]))
                m_vVisible.push_back(m_vAll[i]);

    DeletedFilesOrder order;
    order.iColumn = m_iSortColumn;
    order.bAscending = m_bSortAscending;
    std::sort(m_vVisible.begin(), m_vVisible.end(), order);

    SetItemCount((long) m_vVisible.size());
    if (!setSelected.empty()) {
        std::vector<long> vReselect;
        for (size_t i = 0; i < m_vVisible.size(); i++)
            if (setSelected.count(m_vVisible[i]))
                vReselect.push_back((long) i);

        if (!vReselect.empty() && vReselect.size() == m_vVisible.size())
            SelectAll(true); // e.g. after "Select all" - one message instead of one per row
        else
            for (size_t i = 0; i < vReselect.size(); i++)
                SelectItem(vReselect[i], true);
    }
    Refresh();
}

wxString CDeletedFilesList::OnGetItemText(long lItem, long lColumn) const {
    const t_FileInfo *pFile = GetFile(lItem);
    if (!pFile)
        return wxEmptyString;

    switch (lColumn) {
        case DELETED_FILES_COLUMN_NAME:
            return wxString(pFile->sFilename.c_str());
        case DELETED_FILES_COLUMN_FOLDER:
            return wxString(pFile->sFolder.c_str());
        case DELETED_FILES_COLUMN_SIZE:
            return FormatFileSize(pFile->n64RealSize);
        case DELETED_FILES_COLUMN_MODIFIED:
            return FormatFileTime(pFile->n64Modify);
        case DELETED_FILES_COLUMN_CONTENTS:
            return wxString(ContentLabel(pFile->iContent));
        case DELETED_FILES_COLUMN_RECOVERABILITY:
            if (pFile->iRecoverability == RECOVERABILITY_PARTIAL)
                return wxString::Format("%s (%d%% intact)", RecoverabilityLabel(pFile->iRecoverability), pFile->iPercentIntact);
            return wxString(RecoverabilityLabel(pFile->iRecoverability));
        case DELETED_FILES_COLUMN_STATUS:
            return wxString(FileStatusText(*pFile));
    }
    return wxEmptyString;
}

wxListItemAttr *CDeletedFilesList::OnGetItemAttr(long lItem) const {
    const t_FileInfo *pFile = GetFile(lItem);
    if (pFile && pFile->iRecoverability == RECOVERABILITY_PARTIAL)
        return &m_attrPartial;
    if (pFile && pFile->iRecoverability == RECOVERABILITY_OVERWRITTEN)
        return &m_attrOverwritten;
    return NULL;
}
