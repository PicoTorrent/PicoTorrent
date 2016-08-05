#include <picotorrent/client/ui/property_sheets/details/trackers_page.hpp>

#include <picotorrent/client/i18n/translator.hpp>
#include <picotorrent/client/ui/controls/list_view.hpp>
#include <picotorrent/client/ui/resources.hpp>
#include <picotorrent/client/ui/scaler.hpp>
#include <picotorrent/common/string_operations.hpp>
#include <picotorrent/core/torrent.hpp>
#include <picotorrent/core/tracker.hpp>
#include <picotorrent/core/tracker_status.hpp>

#include <cassert>
#include <chrono>
#include <shlwapi.h>
#include <strsafe.h>

#define CTX_ADD 100
#define CTX_REMOVE 101

#define LIST_COLUMN_URL    1
#define LIST_COLUMN_STATUS 2
#define LIST_COLUMN_UPDATE 3
#define LIST_COLUMN_PEERS  4
#define LIST_COLUMN_SCRAPE 5

using picotorrent::client::ui::controls::list_view;
using picotorrent::client::ui::property_sheets::details::trackers_page;
using picotorrent::client::ui::scaler;
using picotorrent::common::to_string;
using picotorrent::common::to_wstring;
using picotorrent::core::torrent;
using picotorrent::core::tracker;
using picotorrent::core::tracker_status;

struct trackers_page::tracker_state
{
    tracker_state(const tracker &t)
        : tracker(t)
    {
    }

    tracker tracker;
    tracker_status status;
    bool dirty;
};

trackers_page::trackers_page()
{
    set_flags(PSP_USETITLE);
    set_instance(GetModuleHandle(NULL));
    set_template_id(IDD_DETAILS_TRACKERS);
    set_title(TR("trackers"));
}

trackers_page::~trackers_page()
{
}

void trackers_page::refresh(const std::shared_ptr<torrent> &torrent)
{
    for (tracker_state &ts : trackers_)
    {
        ts.dirty = false;
    }

    for (const tracker &t : torrent->get_trackers())
    {
        tracker_status &ts = torrent->get_tracker_status(t.url());

        // TODO: std::find_if uses moderate CPU here, and in a loop as well. maybe a std::map is better for peers_.
        auto &item = std::find_if(trackers_.begin(), trackers_.end(), [t](const tracker_state &ts) { return t.url() == ts.tracker.url(); });

        if (item != trackers_.end())
        {
            item->status = ts;
            item->tracker = t;
            item->dirty = true;
        }
        else
        {
            tracker_state state(t);
            state.dirty = true;
            state.status = ts;
            trackers_.push_back(state);
        }
    }

    trackers_.erase(std::remove_if(trackers_.begin(), trackers_.end(),
        [](const tracker_state &t)
    {
        return !t.dirty;
    }), trackers_.end());

    list_->set_item_count((int)trackers_.size());

    if (sort_items_)
    {
        sort_items_;
    }
}

void trackers_page::on_init_dialog()
{
    HWND hList = GetDlgItem(handle(), ID_DETAILS_TRACKERS_LIST);
    list_ = std::make_unique<list_view>(hList);

    list_->add_column(LIST_COLUMN_URL,    TR("url"),           scaler::x(240));
    list_->add_column(LIST_COLUMN_STATUS, TR("status"),        scaler::x(100));
    list_->add_column(LIST_COLUMN_UPDATE, TR("next_announce"), scaler::x(100), list_view::number);
    list_->add_column(LIST_COLUMN_PEERS,  TR("peers"),         scaler::x(80),  list_view::number);
    list_->add_column(LIST_COLUMN_SCRAPE, TR("scrape"),        scaler::x(80),  list_view::number);

    list_->load_state("torrent_trackers_list");
    on_destroy().connect([this]()
    {
        list_->save_state("torrent_trackers_list");
    });

    list_->on_display().connect(std::bind(&trackers_page::on_list_display, this, std::placeholders::_1));
    list_->on_item_context_menu().connect(std::bind(&trackers_page::on_trackers_context_menu, this, std::placeholders::_1));
    list_->on_sort().connect(std::bind(&trackers_page::on_list_sort, this, std::placeholders::_1));
}

std::string trackers_page::on_list_display(const std::pair<int, int> &p)
{
    const tracker_state &t = trackers_[p.second];

    switch (p.first)
    {
    case LIST_COLUMN_URL:
        return t.tracker.url();
    case LIST_COLUMN_STATUS:
        switch (t.tracker.status())
        {
        case tracker::not_working:
            return "Not working";
        case tracker::updating:
            return "Updating";
        case tracker::working:
            return "Working";
        case tracker::unknown:
        default:
            return "Unknown";
        }
        break;
    case LIST_COLUMN_UPDATE:
    {
        std::chrono::seconds next = t.tracker.next_announce_in();

        if (next.count() < 0)
        {
            return "-";
        }

        std::chrono::minutes min_left = std::chrono::duration_cast<std::chrono::minutes>(next);
        std::chrono::seconds sec_left = std::chrono::duration_cast<std::chrono::seconds>(next - min_left);

        // Return unknown if more than 60 minutes
        if (min_left.count() >= 60)
        {
            return "-";
        }

        TCHAR t[100];
        StringCchPrintf(
            t,
            ARRAYSIZE(t),
            L"%dm %ds",
            min_left.count(),
            sec_left.count());
        return to_string(t);
    }
    case LIST_COLUMN_PEERS:
        if (t.status.num_peers < 0)
        {
            return "-";
        }

        return std::to_string(t.status.num_peers);
    case LIST_COLUMN_SCRAPE:
    {
        int complete = t.status.scrape_complete;
        int incomplete = t.status.scrape_incomplete;

        TCHAR t[100];
        StringCchPrintf(
            t,
            ARRAYSIZE(t),
            L"%s/%s",
            complete < 0 ? L"-" : std::to_wstring(complete).c_str(),
            incomplete < 0 ? L"-" : std::to_wstring(incomplete).c_str());
        return to_string(t);
    }
    default:
        return "<unknown column>";
    }
}

void trackers_page::on_list_sort(const std::pair<int, int> &p)
{
    list_view::sort_order_t order = (list_view::sort_order_t)p.second;
    bool asc = order == list_view::sort_order_t::asc;

    std::function<bool(const tracker_state&, const tracker_state&)> sort_func;

    switch (p.first)
    {
    case LIST_COLUMN_URL:
        sort_func = [asc](const tracker_state &t1, const tracker_state &t2)
        {
            if (asc) { return t1.tracker.url() < t2.tracker.url(); }
            return t1.tracker.url() > t2.tracker.url();
        };
        break;
    case LIST_COLUMN_STATUS:
        sort_func = [asc](const tracker_state &t1, const tracker_state &t2)
        {
            if (asc) { return t1.tracker.status() < t2.tracker.status(); }
            return t1.tracker.status() > t2.tracker.status();
        };
        break;
    case LIST_COLUMN_UPDATE:
        sort_func = [asc](const tracker_state &t1, const tracker_state &t2)
        {
            if (asc) { return t1.tracker.next_announce_in() < t2.tracker.next_announce_in(); }
            return t1.tracker.next_announce_in() > t2.tracker.next_announce_in();
        };
        break;
    case LIST_COLUMN_PEERS:
        sort_func = [asc](const tracker_state &t1, const tracker_state &t2)
        {
            if (asc) { return t1.status.num_peers < t2.status.num_peers; }
            return t1.status.num_peers > t2.status.num_peers;
        };
        break;
    case LIST_COLUMN_SCRAPE:
        sort_func = [asc](const tracker_state &t1, const tracker_state &t2)
        {
            int t1s = t1.status.scrape_complete + t1.status.scrape_incomplete;
            int t2s = t2.status.scrape_complete + t2.status.scrape_incomplete;

            if (asc) { return t1s < t2s; }
            return t1s > t2s;
        };
        break;
    }

    if (sort_func)
    {
        sort_items_ = [this, sort_func]()
        {
            std::sort(trackers_.begin(), trackers_.end(), sort_func);
        };
        sort_items_();
    }
}

void trackers_page::on_trackers_context_menu(const std::vector<int> &items)
{
    HMENU menu = CreatePopupMenu();

    if (items.size() > 0)
    {
        // Remove
        AppendMenu(menu, MF_STRING, CTX_REMOVE, to_wstring(TR("remove_tracker_s")).c_str());
    }
    else
    {
        // Add
        AppendMenu(menu, MF_STRING, CTX_ADD, to_wstring(TR("add_tracker")).c_str());
    }

    POINT pt;
    GetCursorPos(&pt);

    int res = TrackPopupMenu(
        menu,
        TPM_NONOTIFY | TPM_RETURNCMD,
        pt.x,
        pt.y,
        0,
        list_->handle(),
        NULL);

    switch (res)
    {
    case CTX_ADD:
    {
        on_add_tracker_.emit();
        break;
    }
    case CTX_REMOVE:
    {
        std::vector<std::string> trackers;
        for (int i : items)
        {
            trackers.push_back(trackers_[i].tracker.url());
        }
        on_remove_trackers_.emit(trackers);
        break;
    }
    }
}
