#include "Messenger.hpp"

#include "microprofile.h"

namespace neo {

    Messenger::GlobalReceivers Messenger::mReceivers = {};
    Messenger::GlobalMessages Messenger::mMessages = {};
    Messenger::EntityReceivers Messenger::mEntityReceivers = {};
    Messenger::EntityMessages Messenger::mEntityMessages = {};

    void Messenger::relayMessages(ECS& ecs) {
        MICROPROFILE_SCOPEI("Messenger", "relayMessages()", MP_AUTO);

        /* Send entity-level messages */
        if (mEntityMessages.size()) {
            EntityMessages messageBuffer;
            std::swap(mEntityMessages, messageBuffer);

            for (auto& entityMessages : messageBuffer) {
                ECS::Entity entity = entityMessages.first;
                auto& messages = entityMessages.second;

                auto receivers(mEntityReceivers.find(entity));
                if (receivers != mEntityReceivers.end()) {
                    for (auto& message : messages) {
                        auto& msgType = message.first;
                        auto& messageList = message.second;
                        auto receiversList(receivers->second.find(msgType));

                        if (receiversList != receivers->second.end()) {
                            for (auto& receiverFunc : receiversList->second) {
                                for (auto& msg : messageList) {
                                    receiverFunc(*msg, ecs, entity);
                                }
                            }
                        }
                    }
                }
            }
            messageBuffer.clear();
        }

        /* Send scene-level messages */
        if (mMessages.size()) {
            GlobalMessages messageBuffer;
            std::swap(mMessages, messageBuffer);

            for (auto& message : messageBuffer) {
                std::type_index msgTypeI = message.first;
                auto& messages = message.second;

                auto localReceivers(mReceivers.find(msgTypeI));
                if (localReceivers != mReceivers.end()) {
                    for (auto & receiver : localReceivers->second) {
                        for (auto& msg : messages) {
                            receiver(*msg);
                        }
                    }
                }
            }
            messageBuffer.clear();
        }
    }

    void Messenger::clean() {
        mMessages.clear();
        mReceivers.clear();
    }
}