#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dataview.h>

#include <list>
#include <vector>

namespace libtorrent
{
    struct peer_info;
    struct torrent_status;
}

namespace pt
{
    class PeersViewModel : public wxDataViewVirtualListModel
    {
    public:
        enum Columns
        {
            IP,
            Client,
            Flags,
            DownloadRate,
            UploadRate,
            Progress,
            _Max
        };

        PeersViewModel();

        void Clear();
        void Update(libtorrent::torrent_status const& ts);
        void Sort(int columnId, bool ascending);

    private:
        unsigned int GetColumnCount() const wxOVERRIDE;
        wxString GetColumnType(unsigned int col) const wxOVERRIDE;
        void GetValueByRow(wxVariant &variant, unsigned row, unsigned col) const wxOVERRIDE;
        bool SetValueByRow(const wxVariant &variant, unsigned row, unsigned col) wxOVERRIDE;

        std::vector<libtorrent::peer_info> m_data;
        std::list<Columns> m_sortOrder;
    };
}
