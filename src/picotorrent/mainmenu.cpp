#include "mainmenu.hpp"

#include "addtorrentdlg.hpp"
#include "addtorrentproc.hpp"
#include "buildinfo.hpp"
#include "config.hpp"
#include "environment.hpp"
#include "preferencesdlg.hpp"
#include "sessionstate.hpp"
#include "taskbaricon.hpp"
#include "translator.hpp"

#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_info.hpp>
#include <wx/aboutdlg.h>

using pt::MainMenu;

wxBEGIN_EVENT_TABLE(MainMenu, wxMenuBar)
    EVT_MENU(wxID_ABOUT, MainMenu::OnAbout)
    EVT_MENU(ptID_ADD_TORRENTS, MainMenu::OnAddTorrents)
    EVT_MENU(ptID_ADD_MAGNET_LINK, MainMenu::OnAddMagnetLink)
    EVT_MENU(ptID_VIEW_PREFERENCES, MainMenu::OnViewPreferences)
    EVT_MENU(wxID_EXIT, MainMenu::OnExit)
wxEND_EVENT_TABLE()

MainMenu::MainMenu(std::shared_ptr<pt::SessionState> state,
    std::shared_ptr<pt::Configuration> cfg,
    std::shared_ptr<pt::Environment> env,
    std::shared_ptr<pt::TaskBarIcon> taskBarIcon,
    std::shared_ptr<pt::Translator> translator)
    : wxMenuBar(),
    m_state(state),
    m_cfg(cfg),
    m_env(env),
    m_taskBarIcon(taskBarIcon),
    m_trans(translator)
{
    wxMenu* menuFile = new wxMenu();
    menuFile->Append(ptID_ADD_TORRENTS, i18n(m_trans, "amp_add_torrent"));
    menuFile->Append(ptID_ADD_MAGNET_LINK, i18n(m_trans, "amp_add_magnet_link_s"));
    /*menuFile->AppendSeparator();
    menuFile->Append(wxID_ANY, i18n(m_trans, "amp_create_torrent"));*/
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT, i18n(m_trans, "amp_exit"));

    wxMenu* menuView = new wxMenu();
    menuView->Append(ptID_VIEW_PREFERENCES, i18n(m_trans, "amp_preferences"));

    wxMenu* menuHelp = new wxMenu();
    menuHelp->Append(wxID_ABOUT);

    Append(menuFile, i18n(m_trans, "amp_file"));
    Append(menuView, i18n(m_trans, "amp_view"));
    Append(menuHelp, i18n(m_trans, "amp_help"));
}

void MainMenu::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("PicoTorrent");
    aboutInfo.SetVersion(BuildInfo::Version());
    aboutInfo.SetDescription(m_trans->Translate("picotorrent_description"));
    aboutInfo.SetCopyright("(C) 2015-2018");
    aboutInfo.SetWebSite("https://picotorrent.org");
    aboutInfo.AddDeveloper("Viktor Elofsson");

    wxAboutBox(aboutInfo, this->GetFrame());
}

void MainMenu::OnAddMagnetLink(wxCommandEvent& event)
{
    AddTorrentProcedure proc(this->GetFrame(), m_cfg, m_trans, m_state);
    proc.ExecuteMagnet();
}

void MainMenu::OnAddTorrents(wxCommandEvent& event)
{
    AddTorrentProcedure proc(this->GetFrame(), m_cfg, m_trans, m_state);
    proc.Execute();
}

void MainMenu::OnExit(wxCommandEvent& WXUNUSED(event))
{
    this->GetFrame()->Close(true);
}

void MainMenu::OnViewPreferences(wxCommandEvent& WXUNUSED(event))
{
    PreferencesDialog dlg(
        this->GetFrame(),
        m_env,
        m_cfg,
        m_state,
        m_taskBarIcon,
        m_trans);

    dlg.ShowModal();
}
