#include "Messenger.hpp"

#include "GameObject/GameObject.hpp"

namespace neo {

    std::vector<std::tuple<const GameObject *, std::type_index, std::unique_ptr<Message>>> Messenger::messages;
    std::unordered_map<std::type_index, std::vector<std::function<void (const Message &)>>> Messenger::receivers;

    void Messenger::relayMessages() {
        static std::vector<std::tuple<const GameObject *, std::type_index, std::unique_ptr<Message>>> messageBuffer;

        while (messages.size()) {
            /* Corrections for messages sent from receivers */
            std::swap(messages, messageBuffer);

            for (auto & message : messageBuffer) {
                const GameObject * gameObject(std::get<0>(message));
                std::type_index msgTypeI(std::get<1>(message));
                auto & msg(std::get<2>(message));

                /* Send object-level messages */
                if (gameObject) {
                    auto objectReceivers(gameObject->receivers.find(msgTypeI));
                    if (objectReceivers != gameObject->receivers.end()) {
                        for (auto & receiver : objectReceivers->second) {
                            receiver(*msg);
                        }
                    }
                }

                /* Send scene-level messages */
                auto localReceivers(receivers.find(msgTypeI));
                if (localReceivers != receivers.end()) {
                    for (auto & receiver : localReceivers->second) {
                        receiver(*msg);
                    }
                }
            }
        }

        messageBuffer.clear();
    }
}