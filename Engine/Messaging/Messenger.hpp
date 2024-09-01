#pragma once

#include "Message.hpp"

#include "Util/Util.hpp"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <functional>
#include <mutex>

#include <ext/entt_incl.hpp>
#include <entt/signal/dispatcher.hpp>

namespace neo {

    class ECS;

    class Messenger {
        public:
            Messenger() = default;
            ~Messenger() = default;
            Messenger(const Messenger&) = delete;
            Messenger& operator=(const Messenger&) = delete;

            template <typename MsgT, typename... Args> static void sendMessage(Args &&... args);
            template<typename MsgT, auto Func, typename Caller> static void addReceiver(Caller &&caller);
            template<typename MsgT, auto Func, typename Caller> static void removeReceiver(Caller &&caller);
            template<typename MsgT, typename Caller> static void removeReceiver(Caller &&caller);

            static void relayMessages(ECS& ecs);
            static void clean();

        private:
            static std::mutex mDisaptcherMutex;
            static entt::dispatcher mDispatcher;
    };

    template <typename MsgT, typename... Args>
    void Messenger::sendMessage(Args &&... args) {
        std::lock_guard<std::mutex> lock(mDisaptcherMutex);
        mDispatcher.enqueue<MsgT>(std::forward<Args>(args)...);
    }

    template<typename MsgT, auto Func, typename Caller> 
    void Messenger::addReceiver(Caller &&caller) {
        std::lock_guard<std::mutex> lock(mDisaptcherMutex);
        mDispatcher.sink<MsgT>().connect<Func>(caller);
    }
    
    template<typename MsgT, auto Func, typename Caller> 
    void Messenger::removeReceiver(Caller&& caller) {
        std::lock_guard<std::mutex> lock(mDisaptcherMutex);
        mDispatcher.sink<MsgT>().disconnect<Func>(caller);
    }

    template<typename MsgT, typename Caller> 
    void Messenger::removeReceiver(Caller&& caller) {
        std::lock_guard<std::mutex> lock(mDisaptcherMutex);
        mDispatcher.sink<MsgT>().disconnect(caller);
    }
}
