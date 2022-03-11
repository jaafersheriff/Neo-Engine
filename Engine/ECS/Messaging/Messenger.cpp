#include "Messenger.hpp"

#include <microprofile.h>

namespace neo {

    std::vector<std::tuple<std::type_index, std::unique_ptr<Message>>> Messenger::mMessages = {};
    std::unordered_map<std::type_index, std::vector<ReceiverFunc>> Messenger::mReceivers = {};

    void Messenger::relayMessages(ECS& ecs) {
        MICROPROFILE_SCOPEI("Messenger", "relayMessages()", MP_AUTO);
        static std::vector<std::tuple<std::type_index, std::unique_ptr<Message>>> messageBuffer;

        if (mMessages.size()) {
            /* Corrections for messages sent from receivers */
            std::swap(mMessages, messageBuffer);

            for (auto&&[type, message] : messageBuffer) {

                /* Send scene-level messages */
                auto localReceivers(mReceivers.find(type));
                if (localReceivers != mReceivers.end()) {
                    for (auto & receiver : localReceivers->second) {
                        receiver(*message, ecs);
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