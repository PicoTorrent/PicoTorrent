#pragma once

#include <functional>
#include <map>
#include <memory>
#include <thread>
#include <vector>
#include <windows.h>

#include <picotorrent/common.hpp>
#include <picotorrent/core/signals/signal.hpp>

namespace libtorrent
{
    class session;
    class sha1_hash;
    struct settings_pack;
    class torrent_info;
}

namespace picotorrent
{
namespace core
{
    class add_request;
    class timer;
    class torrent;

    class session
    {
    public:
        DLL_EXPORT session();
        DLL_EXPORT ~session();

        DLL_EXPORT void add_torrent(const std::shared_ptr<add_request> &add);

        DLL_EXPORT void load(HWND hWnd);
        DLL_EXPORT void unload();

        DLL_EXPORT void notify();

        DLL_EXPORT void reload_settings();
        DLL_EXPORT void remove_torrent(const std::shared_ptr<torrent> &torrent, bool remove_data = false);

        DLL_EXPORT core::signals::signal_connector<void, const std::shared_ptr<torrent>&>& on_torrent_added();
        DLL_EXPORT core::signals::signal_connector<void, const std::shared_ptr<torrent>&>& on_torrent_finished();
        DLL_EXPORT core::signals::signal_connector<void, const std::shared_ptr<torrent>&>& on_torrent_removed();
        DLL_EXPORT core::signals::signal_connector<void, const std::shared_ptr<torrent>&>& on_torrent_updated();

    protected:
        void on_alert_notify();

    private:
        typedef std::unique_ptr<libtorrent::session> session_ptr;
        typedef std::shared_ptr<torrent> torrent_ptr;
        typedef std::map<libtorrent::sha1_hash, torrent_ptr> torrent_map_t;

        std::shared_ptr<libtorrent::settings_pack> get_session_settings();
        void load_state();
        void load_torrents();
        void remove_torrent_files(const torrent_ptr &torrent);
        void save_state();
        void save_torrent(const libtorrent::torrent_info &ti);
        void save_torrents();
        void timer_callback();

        std::unique_ptr<timer> timer_;
        torrent_map_t torrents_;
        session_ptr sess_;

        // Handle to our main window
        HWND hWnd_;

        // Signals
        core::signals::signal<void, const torrent_ptr&> on_torrent_added_;
        core::signals::signal<void, const torrent_ptr&> on_torrent_finished_;
        core::signals::signal<void, const torrent_ptr&> on_torrent_removed_;
        core::signals::signal<void, const torrent_ptr&> on_torrent_updated_;
    };
}
}
