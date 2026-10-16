#include <windows.h>
#include <commctrl.h>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <algorithm>
#include <set>

#include "DeletedFilesList.h"

// order of the "Recoverable" column: most promising first

static int RecoverabilityRank(const t_FileInfo *pFile) {
    switch (pFile->iRecoverability) {
        case RECOVERABILITY_GOOD: return 0;
        case RECOVERABILITY_PARTIAL: return 1;
        case RECOVERABILITY_UNKNOWN: return 2;
        default: return 3; // overwritten
    }
}

struct DeletedFilesOrder {
    int iColumn;
    bool bAscending;

    bool operator()(const t_FileInfo *a, const t_FileInfo *b) const {
        int r = 0;
        switch (iColumn) {
            case DELETED_FILES_COLUMN_SIZE:
                r = (a->n64RealSize < b->n64RealSize) ? -1 : (a->n64RealSize > b->n64RealSize ? 1 : 0);
                break;
            case DELETED_FILES_COLUMN_RECOVERABILITY:
                r = RecoverabilityRank(a) - RecoverabilityRank(b);
                if (!r) // more intact first
                    r = b->iPercentIntact - a->iPercentIntact;
                break;
            case DELETED_FILES_COLUMN_STATUS:
                r = strcmp(a->szBuffer, b->szBuffer);
                break;
        }
        if (!r) // ties (and the name column) - by name, so the order is stable and predictable
            r = _wcsicmp(a->szFilename, b->szFilename);
        if (!r)
            r = a->iRecordId - b->iRecordId;
        return bAscending ? (r < 0) : (r > 0);
    }
};

static wxString FormatSize(LONGLONG llSize) {
    if (llSize < 1024)
        return wxString::Format("%d B", (int) llSize);
    if (llSize < 1024 * 1024)
        return wxString::Format("%.1f KB", llSize / 1024.0);
    if (llSize < 1024LL * 1024 * 1024)
        return wxString::Format("%.1f MB", llSize / (1024.0 * 1024));
    return wxString::Format("%.2f GB", llSize / (1024.0 * 1024 * 1024));
}

CDeletedFilesList::CDeletedFilesList(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
: wxListCtrl(parent, id, pos, size, wxLC_REPORT | wxLC_VIRTUAL | wxBORDER_THEME | wxLC_HRULES | wxLC_VRULES) {
    m_iSortColumn = DELETED_FILES_COLUMN_RECOVERABILITY; // the most useful order: what can still be recovered first
    m_bSortAscending = true;

    InsertColumn(DELETED_FILES_COLUMN_NAME, wxT("File Name"), wxLIST_FORMAT_LEFT, 280);
    InsertColumn(DELETED_FILES_COLUMN_SIZE, wxT("File Size"), wxLIST_FORMAT_RIGHT, 80);
    InsertColumn(DELETED_FILES_COLUMN_RECOVERABILITY, wxT("Recoverable"), wxLIST_FORMAT_LEFT, 150);
    InsertColumn(DELETED_FILES_COLUMN_STATUS, wxT("File Status"), wxLIST_FORMAT_LEFT, 150);

    m_attrPartial.SetTextColour(wxColour(170, 90, 0)); // dark orange
    m_attrOverwritten.SetTextColour(wxColour(150, 150, 150)); // grey

    SetItemCount(0);
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
    if (m_sFilterUpper.IsEmpty())
        m_vVisible = m_vAll;
    else
        for (size_t i = 0; i < m_vAll.size(); i++)
            if (wxString(m_vAll[i]->szFilename).Upper().Contains(m_sFilterUpper))
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
            return wxString(pFile->szFilename);
        case DELETED_FILES_COLUMN_SIZE:
            return FormatSize(pFile->n64RealSize);
        case DELETED_FILES_COLUMN_RECOVERABILITY:
            if (pFile->iRecoverability == RECOVERABILITY_PARTIAL)
                return wxString::Format("%s (%d%% intact)", RecoverabilityLabel(pFile->iRecoverability), pFile->iPercentIntact);
            return wxString(RecoverabilityLabel(pFile->iRecoverability));
        case DELETED_FILES_COLUMN_STATUS:
            return wxString(pFile->szBuffer);
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
