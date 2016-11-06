#pragma once

#include <memory>

namespace Net
{
    class HttpClient;
    struct HttpResponse;
}

class IPicoTorrent;

namespace Controllers
{
    class CheckForUpdateController
    {
    public:
        CheckForUpdateController(std::shared_ptr<IPicoTorrent> pico);
        void Execute(bool forced = false);

    private:
        void OnHttpResponse(Net::HttpResponse httpResponse, bool forced);

        std::unique_ptr<Net::HttpClient> m_httpClient;
        std::shared_ptr<IPicoTorrent> m_pico;
    };
}
