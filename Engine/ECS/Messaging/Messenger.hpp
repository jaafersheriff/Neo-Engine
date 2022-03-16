#pragma once

#include "ECS/ECS.hpp"
#include "Message.hpp"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <functional>
#include <optional>

namespace neo {

    class Messenger {
        public:
            Messenger() = default;
            ~Messenger() = default;
            Messenger(const Messenger&) = delete;
            Messenger& operator=(const Messenger&) = delete;

            /* Sends out a message for any receivers of that message type to pick up
             * If gameObject is not null, first sends the message locally to receivers of only that object. */
            template <typename MsgT, typename... Args> static void sendMessage(Args &&... args);
            template <typename MsgT, typename... Args> static void sendMessage(ECS::Entity entity, Args &&... args);

            /* Adds a receiver for a message type. If gameObject is null, the function will be called for all messages 
            of that type. If gameObject is not null, the function will be called for only messages of that type sent to that object */
            template <typename MsgT> static void addReceiver(const ReceiverFunc&);
            template <typename MsgT> static void addReceiver(ECS::Entity entity, const EntityReceiverFunc&);

            static void relayMessages(ECS& ecs);
            static void clean();

        private:
            using GlobalReceivers = std::unordered_map<std::type_index, std::vector<ReceiverFunc>>;
            static GlobalReceivers mReceivers;
            using GlobalMessages = std::unordered_map<std::type_index, std::vector<std::unique_ptr<Message>>>;
            static GlobalMessages mMessages;

            using EntityReceivers = std::unordered_map<ECS::Entity, std::unordered_map<std::type_index, std::vector<EntityReceiverFunc>>>;
            static EntityReceivers mEntityReceivers;
            using EntityMessages = std::unordered_map<ECS::Entity, GlobalMessages>;
            static EntityMessages mEntityMessages;
    };

    template <typename MsgT, typename... Args> static void Messenger::sendMessage(Args &&... args) {
        static_assert(std::is_base_of<Message, MsgT>::value, "MsgT must be a message type");
        auto key = std::type_index(typeid(MsgT));
        mMessages[key].emplace_back(std::make_unique<MsgT>(std::forward<Args>(args)...));
    }

    template <typename MsgT, typename... Args> static void Messenger::sendMessage(ECS::Entity entity, Args &&... args) {
        static_assert(std::is_base_of<Message, MsgT>::value, "MsgT must be a message type");
        if (mMessages.find(entity) == mMessages.end()) {
            mMessages[entity] = {};
        }
        mMessages[entity].emplace_back({ typeid(MsgT), std::make_unique<MsgT>(std::forward<Args>(args)...) });
    }

    template <typename MsgT> static void Messenger::addReceiver(const ReceiverFunc& func) {
        static_assert(std::is_base_of<Message, MsgT>::value, "MsgT must be a message type");
        auto key = std::type_index(typeid(MsgT));
        if (mReceivers.find(key) == mReceivers.end()) {
            mReceivers[key] = {};
        }
        mReceivers[key].emplace_back(func);
    }

    template <typename MsgT> static void Messenger::addReceiver(ECS::Entity entity, const EntityReceiverFunc& func) {
        static_assert(std::is_base_of<Message, MsgT>::value, "MsgT must be a message type");
        if (mReceivers.find(entity) == mReceivers.end()) {
            mReceivers[entity] = {};
        }
        mReceivers[entity].emplace_back({ typeid(MsgT), func });

    }
}
