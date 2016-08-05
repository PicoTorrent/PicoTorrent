#pragma once

#include <memory>
#include <string>

namespace picotorrent
{
namespace core
{
    class torrent;
}
namespace common
{
namespace ws
{
namespace messages
{
    class torrent_finished_message
    {
    public:
        torrent_finished_message(const std::shared_ptr<core::torrent> &torrent);
        std::string serialize();

    private:
        std::shared_ptr<core::torrent> torrent_;
    };
}
}
}
}
