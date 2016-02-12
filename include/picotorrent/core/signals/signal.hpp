#pragma once

#include <algorithm>
#include <functional>
#include <picotorrent/common.hpp>
#include <vector>

namespace picotorrent
{
namespace core
{
namespace signals
{
    template<typename TRet, typename TArg>
    class signal_connector
    {
    public:
        DLL_EXPORT signal_connector()
            : callbacks_()
        {
        }

        DLL_EXPORT signal_connector(const signal_connector& that)
            : callbacks_(that.callbacks_)
        {
        }

        DLL_EXPORT void connect(const std::function<TRet(TArg)> &callback)
        {
            callback_item item{ callback };
            callbacks_.push_back(item);
        }

        DLL_EXPORT void disconnect(const std::function<TRet(TArg)> &callback)
        {
            auto it = std::find_if(callbacks_.begin(), callbacks_.end(),
                [callback](callback_item &item)
            {
                return item.callback = callback;
            });

            if (it != callbacks_.end())
            {
                callbacks_.erase(it);
            }
        }

    protected:
        class callback_item
        {
        public:
            std::function<TRet(TArg)> callback;
        };

        std::vector<callback_item> callbacks_;
    };

    template<typename TRet, typename TArg>
    class signal : public signal_connector<TRet, TArg>
    {
    public:
        DLL_EXPORT std::vector<TRet> emit(TArg args)
        {
            std::vector<TRet> result;
            for (callback_item &item : callbacks_)
            {
                result.push_back(item.callback(args));
            }
            return result;
        }
    };

    template<typename TArg>
    class signal<void, TArg> : public signal_connector<void, TArg>
    {
    public:
        DLL_EXPORT void emit(TArg args)
        {
            for (callback_item &item : callbacks_)
            {
                item.callback(args);
            }
        }
    };

    template<typename TRet>
    class signal<TRet, void> : public signal_connector<TRet, void>
    {
    public:
        DLL_EXPORT std::vector<TRet> emit()
        {
            std::vector<TRet> result;
            for (callback_item &item : callbacks_)
            {
                result.push_back(item.callback());
            }
            return result;
        }
    };

    template<>
    class signal<void, void> : public signal_connector<void, void>
    {
    public:
        DLL_EXPORT void emit()
        {
            for (callback_item &item : callbacks_)
            {
                item.callback();
            }
        }
    };
}
}
}
