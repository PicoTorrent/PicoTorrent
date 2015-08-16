#ifndef _PT_SCRIPTING_PYHOST_H
#define _PT_SCRIPTING_PYHOST_H

#include <boost/shared_ptr.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/list.hpp>
#include <boost/python/object.hpp>
#include <string>
#include <vector>

namespace libtorrent
{
    class session;
    class sha1_hash;
    struct torrent_status;
}

class PicoTorrent;
class Session;

class PyHost
{
public:
    PyHost(PicoTorrent* pico);

    void Init();
    void Load();
    void Unload();

    void OnInstanceAlreadyRunning();
    void OnTorrentItemActivated(const libtorrent::sha1_hash& hash);
    void OnTorrentItemSelected(const libtorrent::sha1_hash& hash);

    static void AddTorrent(const libtorrent::torrent_status& status);
    static boost::python::list GetCmdArguments();
    static void Log(std::string message);
    static void UpdateTorrents(boost::python::dict torrents);
    static bool Prompt(std::string message);
    static void SetApplicationStatus(std::string status);

private:
    static PicoTorrent* pico_;
    boost::python::object ns_;
    boost::python::object pt_;
    void* ts_;
};

#endif
