#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <map>
#include <memory>

#include <libtorrent/download_priority.hpp>
#include <wx/dataview.h>

namespace libtorrent
{
    class file_storage;
    class torrent_info;
}

namespace pt
{
    class Translator;

    class FileStorageViewModel : public wxDataViewModel
    {
    public:
        enum Columns
        {
            Name,
            Size,
            Progress,
            Priority,
            _Max
        };

        FileStorageViewModel(std::shared_ptr<Translator> translator);

        // Things we need to override
        unsigned int GetColumnCount() const wxOVERRIDE;
        wxString GetColumnType(unsigned int col) const wxOVERRIDE;
        void GetValue(wxVariant &variant, const wxDataViewItem &item, unsigned int col) const wxOVERRIDE;
        bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col) wxOVERRIDE;
        wxDataViewItem GetParent(const wxDataViewItem &item) const wxOVERRIDE;
        bool IsContainer(const wxDataViewItem &item) const wxOVERRIDE;
        unsigned int GetChildren(const wxDataViewItem &parent, wxDataViewItemArray &array) const wxOVERRIDE;

        void ClearNodes();
        std::vector<int> GetFileIndices(wxDataViewItem&);
        wxDataViewItem GetRootItem();
        void RebuildTree(std::shared_ptr<const libtorrent::torrent_info> ti);
        void UpdatePriorities(const std::vector<libtorrent::download_priority_t>& priorities);
        void UpdateProgress(std::vector<int64_t> const& progress);

    private:
        struct Node
        {
            std::string name;
            int64_t size;
            int index;
            libtorrent::download_priority_t priority;
            float progress;

            std::shared_ptr<Node> parent;
            std::map<std::string, std::shared_ptr<Node>> children;
        };

        void FillIndices(Node* node, std::vector<int>& indices);
        wxIcon GetIconForFile(std::string const& fileName) const;

        wxIcon m_folderIcon;
        std::shared_ptr<Translator> m_trans;
        std::shared_ptr<Node> m_root;
        std::map<int, std::shared_ptr<Node>> m_map;
        std::map<std::string, wxIcon> m_icons;
    };
}
