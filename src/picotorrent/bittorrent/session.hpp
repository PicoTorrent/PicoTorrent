#pragma once

#include <map>
#include <memory>
#include <thread>
#include <unordered_set>

#include <libtorrent/fwd.hpp>
#include <libtorrent/info_hash.hpp>
#include <libtorrent/session_types.hpp>
#include <wx/wx.h>

#include "sessionstatistics.hpp"
#include "torrentstatistics.hpp"

template<typename T>
class PicoCommandEvent : public wxCommandEvent
{
public:
    PicoCommandEvent(wxEventType commandType = wxEVT_NULL, int id = 0)
        : wxCommandEvent(commandType, id) { }

    PicoCommandEvent(const PicoCommandEvent& event)
        : wxCommandEvent(event)
    {
        this->SetData(event.GetData());
    }

    // Required for sending with wxPostEvent()
    wxEvent* Clone() const { return new PicoCommandEvent(*this); }

    T GetData() const { return m_data; }
    void SetData(const T& s) { m_data = s; }

private:
    T m_data;
};

namespace pt { namespace BitTorrent { class TorrentHandle; } }

namespace pt { namespace BitTorrent { typedef PicoCommandEvent<pt::BitTorrent::SessionStatistics> SessionStatisticsEvent; } }
namespace pt { namespace BitTorrent { typedef PicoCommandEvent<libtorrent::info_hash_t> InfoHashEvent; } }
namespace pt { namespace BitTorrent { typedef PicoCommandEvent<std::shared_ptr<libtorrent::torrent_info>> MetadataFoundEvent; } }
namespace pt { namespace BitTorrent { typedef PicoCommandEvent<pt::BitTorrent::TorrentStatistics> TorrentStatisticsEvent; } }
namespace pt { namespace BitTorrent { typedef PicoCommandEvent<std::vector<pt::BitTorrent::TorrentHandle*>> TorrentsUpdatedEvent; } }

wxDECLARE_EVENT(ptEVT_SESSION_DEBUG_MESSAGE, wxCommandEvent);
wxDECLARE_EVENT(ptEVT_SESSION_STATISTICS, pt::BitTorrent::SessionStatisticsEvent);
wxDECLARE_EVENT(ptEVT_TORRENT_ADDED, wxCommandEvent);
wxDECLARE_EVENT(ptEVT_TORRENT_FINISHED, wxCommandEvent);
wxDECLARE_EVENT(ptEVT_TORRENT_METADATA_FOUND, pt::BitTorrent::MetadataFoundEvent);
wxDECLARE_EVENT(ptEVT_TORRENT_REMOVED, pt::BitTorrent::InfoHashEvent);
wxDECLARE_EVENT(ptEVT_TORRENT_STATISTICS, pt::BitTorrent::TorrentStatisticsEvent);
wxDECLARE_EVENT(ptEVT_TORRENTS_UPDATED, pt::BitTorrent::TorrentsUpdatedEvent);

typedef void (wxEvtHandler::* SessionStatisticsEventFunction)(pt::BitTorrent::SessionStatisticsEvent&);
typedef void (wxEvtHandler::* InfoHashEventFunction)(pt::BitTorrent::InfoHashEvent&);
typedef void (wxEvtHandler::* TorrentStatisticsEventFunction)(pt::BitTorrent::TorrentStatisticsEvent&);

#define SessionStatisticsEventHandler(func) wxEVENT_HANDLER_CAST(SessionStatisticsEventFunction, func)
#define InfoHashEventHandler(func) wxEVENT_HANDLER_CAST(InfoHashEventFunction, func)
#define TorrentStatisticsEventHandler(func) wxEVENT_HANDLER_CAST(TorrentStatisticsEventFunction, func)

namespace pt
{
namespace Core
{
    class Configuration;
    class Database;
    class Environment;
}
namespace BitTorrent
{
    class Session : public wxEvtHandler
    {
    public:
        friend class TorrentHandle;

        Session(wxEvtHandler* parent, std::shared_ptr<pt::Core::Database> db, std::shared_ptr<pt::Core::Configuration> cfg, std::shared_ptr<pt::Core::Environment> env);
        virtual ~Session();

        void AddMetadataSearch(std::vector<libtorrent::info_hash_t> const& hashes);
        void AddTorrent(libtorrent::add_torrent_params const& params);
        bool HasTorrent(libtorrent::info_hash_t const& hash);
        void ReloadSettings();
        void RemoveMetadataSearch(std::vector<libtorrent::info_hash_t> const& hashes);
        void RemoveTorrent(TorrentHandle* handle, libtorrent::remove_flags_t flags = {});

    private:
        enum
        {
            ptID_TIMER_SESSION = 1000,
            ptID_TIMER_RESUME_DATA
        };

        bool IsSearching(libtorrent::info_hash_t hash);
        bool IsSearching(libtorrent::info_hash_t hash, libtorrent::info_hash_t& result);
        void LoadIPFilter(std::string const& filePath);
        void LoadTorrents();
        void OnAlert();
        void OnSaveResumeDataTimer(wxTimerEvent&);
        void PauseAfterRecheck(TorrentHandle*);
        void RemoveMetadataHandle(libtorrent::info_hash_t hash);
        void SaveState();
        void SaveTorrents();
        void UpdateMetadataHandle(libtorrent::info_hash_t hash, libtorrent::torrent_handle handle);
        void UpdateTorrentLabel(TorrentHandle*);

        wxEvtHandler* m_parent;
        wxTimer* m_timer;
        wxTimer* m_resumeDataTimer;
        
        std::unique_ptr<libtorrent::session> m_session;
        std::shared_ptr<Core::Database> m_db;
        std::shared_ptr<Core::Configuration> m_cfg;
        std::shared_ptr<Core::Environment> m_env;
        std::thread m_filterLoader;

        std::map<libtorrent::info_hash_t, TorrentHandle*> m_pauseAfterRecheck;
        std::map<libtorrent::info_hash_t, TorrentHandle*> m_torrents;
        std::unordered_set<libtorrent::info_hash_t> m_metadataRemoving;
        std::map<libtorrent::info_hash_t, libtorrent::torrent_handle> m_metadataSearches;
    };
}
}
