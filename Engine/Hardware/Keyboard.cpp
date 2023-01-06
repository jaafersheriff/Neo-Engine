#include "Keyboard.hpp"

#include "GLFW/glfw3.h"
#include "ECS/Messaging/Messenger.hpp"
#include "Util/Util.hpp"

namespace neo {

    void Keyboard::init() {
        Messenger::removeReceiver<ResetKeyboardMessage>(this);
        Messenger::addReceiver<ResetKeyboardMessage, &Keyboard::_onReset>(this);

        Messenger::removeReceiver<KeyPressedMessage>(this);
        Messenger::addReceiver<KeyPressedMessage, &Keyboard::_onKeyPressed>(this);
    }

    bool Keyboard::isKeyPressed(int key) const {
        NEO_ASSERT(key < NUM_KEYS, "Invalid key");
        return mKeyStatus[key];
    }

    void Keyboard::_onKeyPressed(const KeyPressedMessage& msg) {
        NEO_ASSERT(msg.mKey < NUM_KEYS, "Invalid key");
        mKeyStatus[msg.mKey] = msg.mAction >= GLFW_PRESS;
    }

    void Keyboard::_onReset(const ResetKeyboardMessage& msg) {
        NEO_UNUSED(msg);
        for (int key(0); key < NUM_KEYS; ++key) {
            mKeyStatus[key] = false;
        }
    }
}