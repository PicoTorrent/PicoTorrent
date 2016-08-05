#include <picotorrent/common/ws/messages/torrent_finished_message.hpp>

#include <picojson.hpp>
#include <picotorrent/common/ws/serialization/torrent_serializer.hpp>
#include <picotorrent/core/torrent.hpp>

namespace pj = picojson;
using picotorrent::common::ws::messages::torrent_finished_message;
using picotorrent::common::ws::serialization::torrent_serializer;
using picotorrent::core::torrent;

torrent_finished_message::torrent_finished_message(const std::shared_ptr<torrent> &torrent)
    : torrent_(torrent)
{
}

std::string torrent_finished_message::serialize()
{
    pj::object obj;
    obj["type"] = pj::value("torrent_finished");

    pj::object t = torrent_serializer::serialize(torrent_);
    obj["torrent"] = pj::value(t);

    return pj::value(obj).serialize();
}
