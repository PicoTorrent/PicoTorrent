#include <picotorrent/client/controllers/torrent_context_menu_controller.hpp>

#include <algorithm>
#include <picotorrent/client/controllers/move_torrent_controller.hpp>
#include <picotorrent/client/controllers/remove_torrent_controller.hpp>
#include <picotorrent/client/controllers/torrent_details_controller.hpp>
#include <picotorrent/client/ui/main_window.hpp>
#include <picotorrent/client/ui/resources.hpp>
#include <picotorrent/client/ui/torrent_context_menu.hpp>
#include <picotorrent/common/string_operations.hpp>
#include <picotorrent/core/hash.hpp>
#include <picotorrent/core/pal.hpp>
#include <picotorrent/core/session.hpp>
#include <picotorrent/core/torrent.hpp>
#include <shlobj.h>
#include <shlwapi.h>
#include <sstream>

using picotorrent::client::ui::main_window;
using picotorrent::client::ui::torrent_context_menu;
using picotorrent::common::to_wstring;
using picotorrent::core::pal;
using picotorrent::core::session;
using picotorrent::core::torrent;

using picotorrent::client::controllers::move_torrent_controller;
using picotorrent::client::controllers::remove_torrent_controller;
using picotorrent::client::controllers::torrent_context_menu_controller;

typedef std::shared_ptr<torrent> torrent_ptr;

torrent_context_menu_controller::torrent_context_menu_controller(
    const std::shared_ptr<session> &session,
    const std::vector<std::shared_ptr<torrent>> &torrents,
    const std::shared_ptr<main_window> &wnd)
    : sess_(session),
    torrents_(torrents),
    wnd_(wnd)
{
}

void torrent_context_menu_controller::execute(const POINT &p)
{
    torrent_context_menu menu;

    bool all_is_paused = std::all_of(torrents_.begin(), torrents_.end(), [](const torrent_ptr &t) { return t->is_paused(); });
    bool all_is_not_paused = std::all_of(torrents_.begin(), torrents_.end(), [](const torrent_ptr &t) { return !t->is_paused(); });

    if (all_is_paused)
    {
        menu.remove_pause();
        menu.highlight_resume();
    }
    else if(all_is_not_paused)
    {
        menu.remove_resume();
        menu.highlight_pause();
    }

    if (torrents_.size() > 1)
    {
        menu.disable_open_in_explorer();
        menu.disable_queuing();
    }

    switch (menu.show(wnd_->handle(), p))
    {
    case TORRENT_CONTEXT_MENU_PAUSE:
        std::for_each(torrents_.begin(), torrents_.end(), [](const torrent_ptr &t) { t->pause(); });
        break;

    case TORRENT_CONTEXT_MENU_RESUME:
        std::for_each(torrents_.begin(), torrents_.end(),
            [](const torrent_ptr &t)
        {
            if (t->has_error())
            {
                t->clear_error();
            }

            t->resume(false);
        });
        break;

    case TORRENT_CONTEXT_MENU_RESUME_FORCE:
        std::for_each(torrents_.begin(), torrents_.end(),
            [](const torrent_ptr &t)
        {
            if (t->has_error())
            {
                t->clear_error();
            }

            t->resume(true);
        });
        break;

    case TORRENT_CONTEXT_MENU_MOVE:
    {
        move_torrent_controller move_controller(wnd_, torrents_);
        move_controller.execute();
        break;
    }

    case TORRENT_CONTEXT_MENU_QUEUE_UP:
        torrents_[0]->queue_up();
        break;

    case TORRENT_CONTEXT_MENU_QUEUE_DOWN:
        torrents_[0]->queue_down();
        break;

    case TORRENT_CONTEXT_MENU_QUEUE_TOP:
        torrents_[0]->queue_top();
        break;

    case TORRENT_CONTEXT_MENU_QUEUE_BOTTOM:
        torrents_[0]->queue_bottom();
        break;

    case TORRENT_CONTEXT_MENU_REMOVE:
    {
        remove_torrent_controller remove_controller(wnd_, sess_, torrents_);
        remove_controller.execute();
        break;
    }

    case TORRENT_CONTEXT_MENU_REMOVE_DATA:
    {
        remove_torrent_controller remove_controller(wnd_, sess_, torrents_);
        remove_controller.execute(true);
        break;
    }

    case TORRENT_CONTEXT_MENU_COPY_MAGNET:
    {
        break;
    }

    case TORRENT_CONTEXT_MENU_COPY_SHA:
    {
        std::stringstream ss;

        for (uint8_t i = 0; i < torrents_.size(); i++)
        {
            if (i > 0)
            {
                ss << ",";
            }

            ss << torrents_[i]->info_hash()->to_string();
        }
        copy_to_clipboard(ss.str());
        break;
    }
    case TORRENT_CONTEXT_MENU_OPEN_IN_EXPLORER:
    {
        open_and_select_item(
            torrents_[0]->save_path(),
            torrents_[0]->name());
        break;
    }

    case TORRENT_CONTEXT_MENU_DETAILS:
    {
        torrent_details_controller details_controller(wnd_, torrents_[0]);
        details_controller.execute();
        break;
    }
    }
}

void torrent_context_menu_controller::copy_to_clipboard(const std::string &text)
{
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    memcpy(GlobalLock(hMem), text.c_str(), text.size());
    GlobalUnlock(hMem);

    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}

void torrent_context_menu_controller::open_and_select_item(const std::string &path, const std::string &item)
{
    std::string full_path = pal::combine_paths(path, item);
    LPITEMIDLIST il = ILCreateFromPath(to_wstring(full_path).c_str());

    SHOpenFolderAndSelectItems(
        il,
        0,
        0,
        0);

    ILFree(il);
}
