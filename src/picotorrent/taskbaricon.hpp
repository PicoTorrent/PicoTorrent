#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <memory>
#include <wx/taskbar.h>

namespace pt
{
    struct SessionState;
    class Translator;

    class TaskBarIcon : public wxTaskBarIcon
    {
    public:
        TaskBarIcon(wxFrame* parent,
            std::shared_ptr<Translator> translator,
            std::shared_ptr<SessionState> state);

    private:
        enum
        {
            ptID_ADD_TORRENT = wxID_HIGHEST,
            ptID_ADD_MAGNET_LINK,
            ptID_PREFERENCES
        };

        wxDECLARE_EVENT_TABLE();

        wxMenu* CreatePopupMenu() wxOVERRIDE;

        void OnAddTorrent(wxCommandEvent&);
        void OnAddMagnetLink(wxCommandEvent&);
        void OnLeftButtonDClick(wxTaskBarIconEvent&);
        void OnViewPreferences(wxCommandEvent&);

        wxFrame* m_parent;
        std::shared_ptr<SessionState> m_state;
        std::shared_ptr<Translator> m_trans;
    };
}
