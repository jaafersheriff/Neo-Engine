#include "Messenger.hpp"

#include "ECS/GameObject.hpp"

#include "microprofile.h"

namespace neo {

    std::vector<std::tuple<const GameObject*, std::type_index, std::unique_ptr<Message>>> Messenger::mMessages = {};
    std::unordered_map<std::type_index, std::vector<ReceiverFunc>> Messenger::mReceivers = {};

    void Messenger::relayMessages(ECS& ecs) {
        MICROPROFILE_SCOPEI("Messenger", "relayMessages()", MP_AUTO);
        static std::vector<std::tuple<const GameObject *, std::type_index, std::unique_ptr<Message>>> messageBuffer;

        if (mMessages.size()) {
            /* Corrections for messages sent from receivers */
            std::swap(mMessages, messageBuffer);

            for (auto & message : messageBuffer) {
                const GameObject * gameObject(std::get<0>(message));
                std::type_index msgTypeI(std::get<1>(message));
                auto & msg(std::get<2>(message));

                /* Send object-level messages */
                if (gameObject) {
                    auto objectReceivers(gameObject->mReceivers.find(msgTypeI));
                    if (objectReceivers != gameObject->mReceivers.end()) {
                        for (auto & receiver : objectReceivers->second) {
                            receiver(*msg, ecs);
                        }
                    }
                }

                /* Send scene-level messages */
                auto localReceivers(mReceivers.find(msgTypeI));
                if (localReceivers != mReceivers.end()) {
                    for (auto & receiver : localReceivers->second) {
                        receiver(*msg, ecs);
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