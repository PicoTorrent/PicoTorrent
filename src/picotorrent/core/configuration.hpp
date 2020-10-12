#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <loguru.hpp>
#include <nlohmann/json.hpp>

namespace pt
{
namespace Core
{
    class Database;
    class Environment;

    class Configuration
    {
    public:
        struct DhtBootstrapNode
        {
            int32_t id;
            std::string hostname;
            int32_t port;
        };

        struct Label
        {
            Label() : id(-1), colorEnabled(false), savePathEnabled(false), applyFilterEnabled(false) {}
            int32_t id;
            std::string name;
            std::string color;
            bool colorEnabled;
            std::string savePath;
            bool savePathEnabled;
            std::string applyFilter;
            bool applyFilterEnabled;
        };

        struct ListenInterface
        {
            int32_t id;
            std::string address;
            int32_t port;
        };

        enum ConnectionProxyType
        {
            None,
            SOCKS4,
            SOCKS5,
            SOCKS5_Password,
            HTTP,
            HTTP_Password
        };

        enum class WindowState
        {
            Normal = 0,
            Minimized = 1,
            Hidden = 2,
            Maximized = 3
        };

        Configuration(std::shared_ptr<Database> db);
        ~Configuration();

        template<typename T>
        std::optional<T> Get(std::string const& key)
        {
            std::string val;
            if (!GetValue(key, val)
                || val.empty())
            {
                return std::nullopt;
            }

            try
            {
                return nlohmann::json::parse(val).get<T>();
            }
            catch (nlohmann::json::exception const& ex)
            {
                LOG_F(WARNING, "Failed to parse setting %s: %s (%s)", key.c_str(), val.c_str(), ex.what());
            }

            return std::nullopt;
        }

        template<typename T>
        void Set(std::string const& key, T const& value)
        {
            SetValue(key, nlohmann::json(value).dump());
        }

        std::vector<DhtBootstrapNode> GetDhtBootstrapNodes();

        // Labels
        std::vector<Label> GetLabels();
        void DeleteLabel(int32_t id);
        void UpsertLabel(Label const& label);

        std::vector<ListenInterface> GetListenInterfaces();
        void DeleteListenInterface(int id);
        void UpsertListenInterface(ListenInterface const& iface);

        void RestoreDefaults();

    private:
        bool GetValue(std::string const& key, std::string& val);
        void SetValue(std::string const& key, std::string const& val);

        std::shared_ptr<Database> m_db;
    };
}
}
