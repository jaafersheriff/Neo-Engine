#pragma once

#include "Message.hpp"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <functional>

namespace neo {

    class GameObject;

    class Messenger {

        public:
            /* Sends out a message for any receivers of that message type to pick up
             * If gameObject is not null, first sends the message locally to receivers of only that object. */
            template <typename MsgT, typename... Args> static void sendMessage(const GameObject * gameObject, Args &&... args);

            /* Adds a receiver for a message type. If gameObject is null, the function will be called for all messages 
            of that type. If gameObject is not null, the function will be called for only messages of that type sent to that object */
            template <typename MsgT> static void addReceiver(const GameObject * gameObject, const std::function<void(const Message &)> & func);

            static void relayMessages();

        private:
            static std::vector<std::tuple<const GameObject *, std::type_index, std::unique_ptr<Message>>> mMessages;
            static std::unordered_map<std::type_index, std::vector<std::function<void(const Message &)>>> mReceivers;
    };

    template <typename MsgT, typename... Args>
    void Messenger::sendMessage(const GameObject *gameObject, Args &&... args) {
        static_assert(std::is_base_of<Message, MsgT>::value, "MsgT must be a message type");
        mMessages.emplace_back(gameObject, typeid(MsgT), std::make_unique<MsgT>(std::forward<Args>(args)...));
    }

    template <typename MsgT>
    void Messenger::addReceiver(const GameObject *gameObject, const std::function<void(const Message &)> & func) {
        static_assert(std::is_base_of<Message, MsgT>::value, "MsgT must be a message type");

        auto & receiver =  gameObject ? const_cast<GameObject *>(gameObject)->mReceivers : mReceivers;
        receiver[std::type_index(typeid(MsgT))].emplace_back(func);

    }

}
