#pragma once

#include <picotorrent/client/ui/property_sheets/property_sheet_page.hpp>
#include <string>
#include <vector>

namespace picotorrent
{
namespace core
{
    class torrent;
    class tracker;
}
namespace client
{
namespace ui
{
namespace controls
{
    class list_view;
}
namespace property_sheets
{
namespace details
{
    class trackers_page : public property_sheet_page
    {
    public:
        trackers_page();
        ~trackers_page();

        void refresh(const std::shared_ptr<core::torrent> &torrent);

    protected:
        void on_init_dialog();

    private:
        std::wstring on_list_display(const std::pair<int, int> &p);

        struct tracker_state;
        std::unique_ptr<controls::list_view> list_;
        std::vector<tracker_state> trackers_;

    };
}
}
}
}
}
