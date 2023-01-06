#pragma once

#include "Message.hpp"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <functional>
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
            static entt::dispatcher mDispatcher;
    };

    template <typename MsgT, typename... Args>
    void Messenger::sendMessage(Args &&... args) {
        mDispatcher.enqueue<MsgT>(std::forward<Args>(args)...);
    }

    template<typename MsgT, auto Func, typename Caller> 
    void Messenger::addReceiver(Caller &&caller) {
        mDispatcher.sink<MsgT>().connect<Func>(caller);

    }
    
    template<typename MsgT, auto Func, typename Caller> 
    void Messenger::removeReceiver(Caller&& caller) {
        mDispatcher.sink<MsgT>().disconnect<Func>(caller);
    }

    template<typename MsgT, typename Caller> 
    void Messenger::removeReceiver(Caller&& caller) {
        mDispatcher.sink<MsgT>().disconnect(caller);
    }
}
