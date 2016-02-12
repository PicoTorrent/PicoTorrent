#include <picotorrent/core/torrent_info.hpp>

#include <picotorrent/core/filesystem/file.hpp>
#include <picotorrent/core/filesystem/path.hpp>
#include <picotorrent/core/logging/log.hpp>

#include <picotorrent/_aux/disable_3rd_party_warnings.hpp>
#include <libtorrent/torrent_info.hpp>
#include <picotorrent/_aux/enable_3rd_party_warnings.hpp>

namespace lt = libtorrent;
namespace fs = picotorrent::core::filesystem;
using picotorrent::core::torrent_info;

torrent_info::torrent_info(const std::vector<char> &buf)
    : info_(std::make_unique<lt::torrent_info>(&buf[0], (int)buf.size()))
{
}

torrent_info::torrent_info(const lt::torrent_info &info)
    : info_(std::make_unique<lt::torrent_info>(info))
{
}

torrent_info::~torrent_info()
{
}

std::shared_ptr<torrent_info> torrent_info::try_load(const fs::path &path)
{
    if (!path.exists())
    {
        return nullptr;
    }

    fs::file f(path);
    std::vector<char> buf;

    try
    {
        f.read_all(buf);
    }
    catch (const std::exception &e)
    {
        LOG(error) << "Error when reading file: " << e.what();
        return nullptr;
    }

    return std::make_shared<torrent_info>(buf);
}

std::string torrent_info::file_path(int index) const
{
    return info_->files().file_path(index);
}

int64_t torrent_info::file_size(int index) const
{
    return info_->files().file_size(index);
}

std::string torrent_info::name()
{
    return info_->name();
}

int torrent_info::num_files() const
{
    return info_->num_files();
}

int64_t torrent_info::total_size()
{
    return info_->total_size();
}
