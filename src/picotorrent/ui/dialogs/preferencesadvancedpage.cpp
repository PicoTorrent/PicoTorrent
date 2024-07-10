#include "preferencesadvancedpage.hpp"

#include <map>

#include <wx/propgrid/manager.h>
#include <wx/propgrid/propgrid.h>

#include "../../core/configuration.hpp"

using pt::Core::Configuration;
using pt::UI::Dialogs::PreferencesAdvancedPage;

struct Property
{
    std::function<wxPGProperty*()> ctor;
    std::string help;
    std::function<void(Configuration*, wxPGProperty*)> get;
    std::function<void(Configuration*, wxPGProperty*)> set;
};

#define MAKE_PROP(t1, t2, t3, key, label, description) \
    { \
        label, \
        { \
            []() { return new wx ##t1 ##Property(label); },\
            description, \
            [](Configuration* cfg, wxPGProperty* prop) { prop->SetValue(cfg->Get<##t3>(key).value()); }, \
            [](Configuration* cfg, wxPGProperty* prop) { cfg->Set(key, prop->GetValue().Get ##t2()); } \
        } \
    }

static std::map<std::string, std::map<std::string, Property>> properties =
{
    {
        "libtorrent",
        {
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.active_checking", "active_checking", "The limit of number of simultaneous checking torrents."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.active_dht_limit", "active_dht_limit", "The max number of torrents to announce to the DHT."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.active_lsd_limit", "active_lsd_limit", "The max number of torrents to announce to the local network over the local service discovery protocol."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.active_tracker_limit", "active_tracker_limit", "The max number of torrents to announce to their trackers."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.allowed_fast_set_size", "allowed_fast_set_size", "The number of allowed pieces to send to peers that supports the fast extensions."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.allow_multiple_connections_per_ip", "allow_multiple_connections_per_ip", "Determines if connections from the same IP address as existing connections should be rejected or not. Rejecting multiple connections from the same IP address will prevent abusive behavior by peers. The logic for determining whether connections are to the same peer is more complicated with this enabled, and more likely to fail in some edge cases. It is not recommended to enable this feature."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.announce_to_all_tiers", "announce_to_all_tiers", "Controls how multi tracker torrents are treated. When this is set to true, one tracker from each tier is announced to. This is the uTorrent behavior. To be compliant with the Multi-tracker specification, set it to false."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.announce_to_all_trackers", "announce_to_all_trackers", "Controls how multi tracker torrents are treated. If this is set to true, all trackers in the same tier are announced to in parallel. If all trackers in tier 0 fails, all trackers in tier 1 are announced as well. If it's set to false, the behavior is as defined by the multi tracker specification."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.anonymous_mode", "anonymous_mode", "When set to true, the client tries to hide its identity to a certain degree. The user-agent will be reset to an empty string (except for private torrents). Trackers will only be used if they are using a proxy server. The listen sockets are closed, and incoming connections will only be accepted through a SOCKS5 or I2P proxy (if a peer proxy is set up and is run on the same machine as the tracker proxy). Since no incoming connections are accepted, NAT-PMP, UPnP, DHT and local peer discovery are all turned off when this setting is enabled. If you're using I2P, it might make sense to enable anonymous mode as well."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.auto_manage_interval", "auto_manage_interval", "auto_manage_interval is the number of seconds between the torrent queue is updated, and rotated."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.auto_manage_prefer_seeds", "auto_manage_prefer_seeds", "If true, prefer seeding torrents when determining which torrents to give active slots to. If false, give preference to downloading torrents."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.auto_scrape_interval", "auto_scrape_interval", "auto_scrape_interval is the number of seconds between scrapes of queued torrents (auto managed and paused torrents). Auto managed torrents that are paused, are scraped regularly in order to keep track of their downloader/seed ratio. This ratio is used to determine which torrents to seed and which to pause."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.auto_scrape_min_interval", "auto_scrape_min_interval", "auto_scrape_min_interval is the minimum number of seconds between any automatic scrape (regardless of torrent). In case there are a large number of paused auto managed torrents, this puts a limit on how often a scrape request is sent."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.checking_mem_usage", "checking_mem_usage", "The number of blocks to keep outstanding at any given time when checking torrents. Higher numbers give faster re-checks but uses more memory. Specified in number of 16 kiB blocks."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.choking_algorithm", "choking_algorithm", "choking_algorithm specifies which algorithm to use to determine how many peers to unchoke. The unchoking algorithm for downloading torrents is always 'tit-for-tat', i.e. the peers we download the fastest from are unchoked. Options are: 0 = fixed_slots_choker, 2 = rate_based_choker, 3 = deprecated_bittyrant_choker"),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.connection_speed", "connection_speed", "connection_speed is the number of connection attempts that are made per second. If a number < 0 is specified, it will default to 200 connections per second. If 0 is specified, it means don't make outgoing connections at all."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.disk_write_mode", "disk_write_mode", "controls whether disk writes will be made through a memory mapped file or via normal write calls. This only affects the mmap_disk_io. When saving to a non-local drive (network share, NFS or NAS) using memory mapped files is most likely inferior. When writing to a local SSD (especially in DAX mode) using memory mapped files likely gives the best performance. Options are: 0 = always_pwrite, 1 = always_mmap_write, 2 = auto_mmap_write"),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.dont_count_slow_torrents", "dont_count_slow_torrents", "If dont_count_slow_torrents is true, torrents without any payload transfers are not subject to the active_seeds and active_downloads limits. This is intended to make it more likely to utilize all available bandwidth, and avoid having torrents that don't transfer anything block the active slots."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.file_pool_size", "file_pool_size", "Sets the upper limit on the total number of files this session will keep open. The reason why files are left open at all is that some anti virus software hooks on every file close, and scans the file for viruses. deferring the closing of the files will be the difference between a usable system and a completely hogged down system. Most operating systems also has a limit on the total number of file descriptors a process may have open."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.hashing_threads", "hashing_threads", "hashing_threads is the number of disk I/O threads to use for piece hash verification. These threads are in addition to the regular disk I/O threads specified by settings_pack::aio_threads. These threads are only used for full checking of torrents. The hash checking done while downloading are done by the regular disk I/O threads. The hasher threads do not only compute hashes, but also perform the read from disk. On storage optimal for sequential access, such as hard drives, this setting should be set to 1, which is also the default."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.inactive_down_rate", "inactive_down_rate", "The download rate limits for a torrent to be considered active by the queuing mechanism. A torrent whose download rate is less than inactive_down_rate and whose upload rate is less than inactive_up_rate for auto_manage_startup seconds, is considered inactive, and another queued torrent may be started. This logic is disabled if dont_count_slow_torrents is false."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.inactive_up_rate", "inactive_up_rate", "The upload rate limits for a torrent to be considered active by the queuing mechanism. A torrent whose download rate is less than inactive_down_rate and whose upload rate is less than inactive_up_rate for auto_manage_startup seconds, is considered inactive, and another queued torrent may be started. This logic is disabled if dont_count_slow_torrents is false."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.inactivity_timeout", "inactivity_timeout", "If a peer is uninteresting and uninterested for longer than this number of seconds, it will be disconnected."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.incoming_starts_queued_torrents", "incoming_starts_queued_torrents", "If a torrent has been paused by the auto managed feature, i.e. the torrent is paused and auto managed, this feature affects whether or not it is automatically started on an incoming connection. The main reason to queue torrents, is not to make them unavailable, but to save on the overhead of announcing to the trackers, the DHT and to avoid spreading one's unchoke slots too thin. If a peer managed to find us, even though we're no in the torrent anymore, this setting can make us start the torrent and serve it."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.initial_picker_threshold", "initial_picker_threshold", "initial_picker_threshold specifies the number of pieces we need before we switch to rarest first picking. The first initial_picker_threshold pieces in any torrent are picked at random , the following pieces are picked in rarest first order."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.listen_queue_size", "listen_queue_size", "listen_queue_size is the value passed in to listen() for the listen socket. It is the number of outstanding incoming connections to queue up while we're not actively waiting for a connection to be accepted. 5 should be sufficient for any normal client. If this is a high performance server which expects to receive a lot of connections, or used in a simulator or test, it might make sense to raise this number. It will not take affect until the listen_interfaces settings is updated."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.max_allowed_in_request_queue", "max_allowed_in_request_queue", "The number of outstanding block requests a peer is allowed to queue up in the client. If a peer sends more requests than this (before the first one has been sent) the last request will be dropped. the higher this is, the faster upload speeds the client can get to a single peer."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.max_failcount", "max_failcount", "max_failcount is the maximum times we try to connect to a peer before stop connecting again. If a peer succeeds, the failure counter is reset. If a peer is retrieved from a peer source (other than DHT) the failcount is decremented by one, allowing another try."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.max_out_request_queue", "max_out_request_queue", "max_out_request_queue is the maximum number of outstanding requests to send to a peer. This limit takes precedence over request_queue_time. i.e. no matter the download speed, the number of outstanding requests will never exceed this limit."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.max_peer_recv_buffer_size", "max_peer_recv_buffer_size", "The max number of bytes a single peer connection's receive buffer is allowed to grow to."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.max_queued_disk_bytes", "max_queued_disk_bytes", "max_queued_disk_bytes is the maximum number of bytes, to be written to disk, that can wait in the disk I/O thread queue. This queue is only for waiting for the disk I/O thread to receive the job and either write it to disk or insert it in the write cache. When this limit is reached, the peer connections will stop reading data from their sockets, until the disk thread catches up. Setting this too low will severely limit your download rate."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.max_rejects", "max_rejects", "max_rejects is the number of piece requests we will reject in a row while a peer is choked before the peer is considered abusive and is disconnected."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.min_reconnect_time", "min_reconnect_time", "The number of seconds to wait to reconnect to a peer. this time is multiplied with the failcount."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.mixed_mode_algorithm", "mixed_mode_algorithm", "The mixed_mode_algorithm determines how to treat TCP connections when there are uTP connections. Since uTP is designed to yield to TCP, there's an inherent problem when using swarms that have both TCP and uTP connections. If nothing is done, uTP connections would often be starved out for bandwidth by the TCP connections. This mode is prefer_tcp. The peer_proportional mode simply looks at the current throughput and rate limits all TCP connections to their proportional share based on how many of the connections are TCP. This works best if uTP connections are not rate limited by the global rate limiter (which they aren't by default). Options are: 0 = prefer_tcp, 1 = peer_proportional"),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.mmap_file_size_cutoff", "mmap_file_size_cutoff", "When using mmap_disk_io, files smaller than this number of blocks will not be memory mapped, but will use normal pread/pwrite operations. This file size limit is specified in 16 kiB blocks."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.no_atime_storage", "no_atime_storage", "no_atime_storage this is a Linux-only option and passes in the O_NOATIME to open() when opening files. This may lead to some disk performance improvements."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.peer_timeout", "peer_timeout", "peer_timeout is the number of seconds the peer connection should wait (for any activity on the peer connection) before closing it due to time out. 120 seconds is specified in the protocol specification. After half the time out, a keep alive message is sent."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.peer_turnover", "peer_turnover", "peer_turnover is the percentage of peers to disconnect every turnover peer_turnover_interval (if we're at the peer limit), this is specified in percent when we are connected to more than limit * peer_turnover_cutoff peers disconnect peer_turnover fraction of the peers. It is specified in percent peer_turnover_interval is the interval (in seconds) between optimistic disconnects if the disconnects happen and how many peers are disconnected is controlled by peer_turnover and peer_turnover_cutoff."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.peer_turnover_cutoff", "peer_turnover_cutoff", "peer_turnover is the percentage of peers to disconnect every turnover peer_turnover_interval (if we're at the peer limit), this is specified in percent when we are connected to more than limit * peer_turnover_cutoff peers disconnect peer_turnover fraction of the peers. It is specified in percent peer_turnover_interval is the interval (in seconds) between optimistic disconnects if the disconnects happen and how many peers are disconnected is controlled by peer_turnover and peer_turnover_cutoff."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.peer_turnover_interval", "peer_turnover_interval", "peer_turnover is the percentage of peers to disconnect every turnover peer_turnover_interval (if we're at the peer limit), this is specified in percent when we are connected to more than limit * peer_turnover_cutoff peers disconnect peer_turnover fraction of the peers. It is specified in percent peer_turnover_interval is the interval (in seconds) between optimistic disconnects if the disconnects happen and how many peers are disconnected is controlled by peer_turnover and peer_turnover_cutoff."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.predictive_piece_announce", "predictive_piece_announce", "If set to > 0, pieces will be announced to other peers before they are fully downloaded (and before they are hash checked). The intention is to gain 1.5 potential round trip times per downloaded piece. When non-zero, this indicates how many milliseconds in advance pieces should be announced, before they are expected to be completed."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.rate_choker_initial_threshold", "rate_choker_initial_threshold", "The rate based choker compares the upload rate to peers against a threshold that increases proportionally by its size for every peer it visits, visiting peers in decreasing upload rate. The number of upload slots is determined by the number of peers whose upload rate exceeds the threshold. This option sets the start value for this threshold. A higher value leads to fewer unchoke slots, a lower value leads to more."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.request_timeout", "request_timeout", "The number of seconds one block (16 kiB) is expected to be received within. If it's not, the block is requested from a different peer."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.seed_choking_algorithm", "seed_choking_algorithm", "seed_choking_algorithm controls the seeding unchoke behavior. i.e. How we select which peers to unchoke for seeding torrents. Since a seeding torrent isn't downloading anything, the tit-for-tat mechanism cannot be used. Options are: 0 = round_robin, 1 = fastest_upload, 2 = anti_leech"),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.seed_time_ratio_limit", "seed_time_ratio_limit", "When a seeding torrent reaches either the share ratio (bytes up / bytes down) or the seed time ratio (seconds as seed / seconds as downloader) or the seed time limit (seconds as seed) it is considered done, and it will leave room for other torrents. These are specified as percentages. Torrents that are considered done will still be allowed to be seeded, they just won't have priority anymore. For more, see queuing."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.send_buffer_low_watermark", "send_buffer_low_watermark", "send_buffer_low_watermark the minimum send buffer target size (send buffer includes bytes pending being read from disk). For good and snappy seeding performance, set this fairly high, to at least fit a few blocks. This is essentially the initial window size which will determine how fast we can ramp up the send rate if the send buffer has fewer bytes than send_buffer_watermark, we'll read another 16 kiB block onto it. If set too small, upload rate capacity will suffer. If set too high, memory will be wasted. The actual watermark may be lower than this in case the upload rate is low, this is the upper limit. the current upload rate to a peer is multiplied by this factor to get the send buffer watermark. The factor is specified as a percentage. i.e. 50 -> 0.5 This product is clamped to the send_buffer_watermark setting to not exceed the max. For high speed upload, this should be set to a greater value than 100. For high capacity connections, setting this higher can improve upload performance and disk throughput. Setting it too high may waste RAM and create a bias towards read jobs over write jobs."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.send_buffer_watermark", "send_buffer_watermark", "send_buffer_low_watermark the minimum send buffer target size (send buffer includes bytes pending being read from disk). For good and snappy seeding performance, set this fairly high, to at least fit a few blocks. This is essentially the initial window size which will determine how fast we can ramp up the send rate if the send buffer has fewer bytes than send_buffer_watermark, we'll read another 16 kiB block onto it. If set too small, upload rate capacity will suffer. If set too high, memory will be wasted. The actual watermark may be lower than this in case the upload rate is low, this is the upper limit. the current upload rate to a peer is multiplied by this factor to get the send buffer watermark. The factor is specified as a percentage. i.e. 50 -> 0.5 This product is clamped to the send_buffer_watermark setting to not exceed the max. For high speed upload, this should be set to a greater value than 100. For high capacity connections, setting this higher can improve upload performance and disk throughput. Setting it too high may waste RAM and create a bias towards read jobs over write jobs."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.send_buffer_watermark_factor", "send_buffer_watermark_factor", "send_buffer_low_watermark the minimum send buffer target size (send buffer includes bytes pending being read from disk). For good and snappy seeding performance, set this fairly high, to at least fit a few blocks. This is essentially the initial window size which will determine how fast we can ramp up the send rate if the send buffer has fewer bytes than send_buffer_watermark, we'll read another 16 kiB block onto it. If set too small, upload rate capacity will suffer. If set too high, memory will be wasted. The actual watermark may be lower than this in case the upload rate is low, this is the upper limit. the current upload rate to a peer is multiplied by this factor to get the send buffer watermark. The factor is specified as a percentage. i.e. 50 -> 0.5 This product is clamped to the send_buffer_watermark setting to not exceed the max. For high speed upload, this should be set to a greater value than 100. For high capacity connections, setting this higher can improve upload performance and disk throughput. Setting it too high may waste RAM and create a bias towards read jobs over write jobs."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.send_not_sent_low_watermark", "send_not_sent_low_watermark", "Specify the not-sent low watermark for socket send buffers. This corresponds to the, Linux-specific, TCP_NOTSENT_LOWAT TCP socket option."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.share_ratio_limit", "share_ratio_limit", "When a seeding torrent reaches either the share ratio (bytes up / bytes down) or the seed time ratio (seconds as seed / seconds as downloader) or the seed time limit (seconds as seed) it is considered done, and it will leave room for other torrents. These are specified as percentages. Torrents that are considered done will still be allowed to be seeded, they just won't have priority anymore. For more, see queuing."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.stop_tracker_timeout", "stop_tracker_timeout", "The number of seconds to wait when sending a stopped message before considering a tracker to have timed out. This is usually shorter, to make the client quit faster. If the value is set to 0, the connections to trackers with the stopped event are suppressed."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.strict_end_game_mode", "strict_end_game_mode", "strict_end_game_mode controls when a block may be requested twice. If this is true, a block may only be requested twice when there's at least one request to every piece that's left to download in the torrent. This may slow down progress on some pieces sometimes, but it may also avoid downloading a lot of redundant bytes. If this is false, libtorrent attempts to use each peer connection to its max, by always requesting something, even if it means requesting something that has been requested from another peer already."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.suggest_mode", "suggest_mode", "suggest_mode controls whether or not libtorrent will send out suggest messages to create a bias of its peers to request certain pieces. The modes are: no_piece_suggestions which will not send out suggest messages and suggest_read_cache which will send out suggest messages for the most recent pieces that are in the read cache. Options are: 0 = no_piece_suggestions, 1 = suggest_read_cache"),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.torrent_connect_boost", "torrent_connect_boost", "torrent_connect_boost is the number of peers to try to connect to immediately when the first tracker response is received for a torrent. This is a boost given to new torrents to accelerate them starting up. The normal connect scheduler is run once every second, this allows peers to be connected immediately instead of waiting for the session tick to trigger connections. This may not be set higher than 255."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.unchoke_slots_limit", "unchoke_slots_limit", "unchoke_slots_limit is the max number of unchoked peers in the session. The number of unchoke slots may be ignored depending on what choking_algorithm is set to. Setting this limit to -1 means unlimited, i.e. all peers will always be unchoked."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.use_parole_mode", "use_parole_mode", "use_parole_mode specifies if parole mode should be used. Parole mode means that peers that participate in pieces that fail the hash check are put in a mode where they are only allowed to download whole pieces. If the whole piece a peer in parole mode fails the hash check, it is banned. If a peer participates in a piece that passes the hash check, it is taken out of parole mode."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.whole_pieces_threshold", "whole_pieces_threshold", "If a whole piece can be downloaded in this number of seconds, or less, the peer_connection will prefer to request whole pieces at a time from this peer. The benefit of this is to better utilize disk caches by doing localized accesses and also to make it easier to identify bad peers if a piece fails the hash check."),
            MAKE_PROP(Int,      Integer,    int,            "libtorrent.announce_ip", "announce_ip", "announce_ip is the ip address passed along to trackers as the &ip= parameter. If left as the default, that parameter is omitted."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.rate_limit_ip_overhead", "rate_limit_ip_overhead", "If set to true, the estimated TCP/IP overhead is drained from the rate limiters, to avoid exceeding the limits with the total traffic."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.enable_outgoing_utp", "enable_outgoing_utp", "Enables outgoing, uTP peer connections. false is disabled and true is enabled. When outgoing connections are disabled, libtorrent will simply not make outgoing peer connections with the specific transport protocol. Disabled incoming peer connections will simply be rejected. These options only apply to peer connections, not tracker- or any other kinds of connections."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.enable_incoming_utp", "enable_incoming_utp", "Enables incoming, uTP peer connections. false is disabled and true is enabled. When outgoing connections are disabled, libtorrent will simply not make outgoing peer connections with the specific transport protocol. Disabled incoming peer connections will simply be rejected. These options only apply to peer connections, not tracker- or any other kinds of connections."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.enable_outgoing_tcp", "enable_outgoing_tcp", "Enables outgoing, TCP peer connections. false is disabled and true is enabled. When outgoing connections are disabled, libtorrent will simply not make outgoing peer connections with the specific transport protocol. Disabled incoming peer connections will simply be rejected. These options only apply to peer connections, not tracker- or any other kinds of connections."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.enable_incoming_tcp", "enable_incoming_tcp", "Enables incoming, TCP peer connections. false is disabled and true is enabled. When outgoing connections are disabled, libtorrent will simply not make outgoing peer connections with the specific transport protocol. Disabled incoming peer connections will simply be rejected. These options only apply to peer connections, not tracker- or any other kinds of connections."),
            MAKE_PROP(Bool,     Bool,       bool,           "libtorrent.dht_privacy_lookups", "dht_privacy_lookups", "When set, perform lookups in a way that is slightly more expensive, but which minimizes the amount of information leaked about you."),
        }
    },
    {
        "PicoTorrent",
        {
            MAKE_PROP(Int,  Integer, int,  "save_resume_data_interval",   "save_resume_data_interval", "The interval (in seconds) between checks to save resume data for torrents. Saving resume data will help keep a current state if (for example) the application exits unexpectedly."),
            MAKE_PROP(Int,  Integer, int,  "ui.torrent_overview.columns", "torrent_overview_columns",  "The number of columns to show in the torrent overview panel."),
            MAKE_PROP(Bool, Bool,    bool, "ui.torrent_overview.show_piece_progress", "torrent_overview_show_piece_progress",  "When set to true, show the piece progress bar in the torrent overview panel.")
        }
    }
};

PreferencesAdvancedPage::PreferencesAdvancedPage(wxWindow* parent, std::shared_ptr<pt::Core::Configuration> cfg)
    : wxPanel(parent),
    m_cfg(cfg)
{
    m_grid = new wxPropertyGridManager(
        this,
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        wxPG_BOLD_MODIFIED | wxPG_SPLITTER_AUTO_CENTER | wxPG_DESCRIPTION | wxPGMAN_DEFAULT_STYLE);

    m_page = m_grid->AddPage();

    for (auto& [category, props] : properties)
    {
        m_page->Append(new wxPropertyCategory(category));

        for (auto& [key, prop] : props)
        {
            auto p = prop.ctor();

            // Set prop value
            prop.get(
                m_cfg.get(),
                p);

            p->SetHelpString(prop.help);
            m_page->Append(p);
        }
    }

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_grid, 1, wxEXPAND | wxALL);

    this->SetSizerAndFit(sizer);
}

void PreferencesAdvancedPage::Save()
{
    for (auto& [cat, props] : properties)
    {
        for (auto& [key, prop] : props)
        {
            prop.set(
                m_cfg.get(),
                m_page->GetProperty(key));
        }
    }
}
