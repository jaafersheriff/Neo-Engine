#include "Keyboard.hpp"

#include "GLFW/glfw3.h"
#include "ECS/Messaging/Messenger.hpp"
#include "Util/Util.hpp"

namespace neo {

    void Keyboard::init() {
        Messenger::addReceiver<ResetKeyboardMessage>([this](const Message& message) {
            NEO_UNUSED(message);
            for (int key(0); key < NUM_KEYS; ++key) {
                mKeyStatus[key] = false;
            }
            });

        Messenger::addReceiver<KeyPressedMessage>([this](const Message& message) {
            const KeyPressedMessage& msg(static_cast<const KeyPressedMessage&>(message));
            NEO_ASSERT(msg.mKey < NUM_KEYS, "Invalid key");
            mKeyStatus[msg.mKey] = msg.mAction >= GLFW_PRESS;
            });
    }

    bool Keyboard::isKeyPressed(int key) const {
        NEO_ASSERT(key < NUM_KEYS, "Invalid key");
        return mKeyStatus[key];
    }
}