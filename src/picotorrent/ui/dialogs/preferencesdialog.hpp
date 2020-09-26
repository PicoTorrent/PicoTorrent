#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <memory>

class wxBookCtrlEvent;
class wxSimplebook;

namespace pt::Core { class Configuration; }

namespace pt::UI::Dialogs
{
    class PreferencesAdvancedPage;
    class PreferencesConnectionPage;
    class PreferencesDownloadsPage;
    class PreferencesGeneralPage;
    class PreferencesProxyPage;
    class PreferencesRssPage;

    class PreferencesDialog : public wxDialog
    {
    public:
        PreferencesDialog(wxWindow* parent, std::shared_ptr<Core::Configuration> cfg);
        virtual ~PreferencesDialog();
        bool WantsRestart() { return m_wantsRestart; }

    private:
        enum
        {
            ptID_BTN_RESTORE_DEFAULTS = wxID_HIGHEST + 1
        };

        void OnOk(wxCommandEvent&);
        void ShowRestartRequiredDialog();

        wxListBox* m_list;
        wxBoxSizer* m_mainSizer;
        wxSimplebook* m_book;

        bool m_wantsRestart;

        PreferencesGeneralPage* m_general;
        PreferencesDownloadsPage* m_downloads;
        PreferencesConnectionPage* m_connection;
        PreferencesProxyPage* m_proxy;
        PreferencesRssPage* m_rss;
        PreferencesAdvancedPage* m_advanced;
    };
}
