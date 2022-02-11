#include "Keyboard.hpp"

#include "GLFW/glfw3.h"
#include "Messaging/Messenger.hpp"
#include "Util/Util.hpp"

namespace neo {

    void Keyboard::init() {
        Messenger::addReceiver<ResetKeyboardMessage>(nullptr, [this](const neo::Message& msg) {
            const ResetKeyboardMessage& m(static_cast<const ResetKeyboardMessage&>(msg));
            NEO_UNUSED(m);

            reset();
        });

        Messenger::addReceiver<KeyPressedMessage>(nullptr, [this](const neo::Message& msg) {
            const KeyPressedMessage& m(static_cast<const KeyPressedMessage&>(msg));

            setKeyStatus(m.mKey, m.mAction);
        });
    }

    bool Keyboard::isKeyPressed(int key) const {
        return mKeyStatus[key] >= GLFW_PRESS;
    }

    void Keyboard::setKeyStatus(int key, int action) {
        mKeyStatus[key] = action;
    }

    void Keyboard::reset() {
        for (int key(0); key < NUM_KEYS; ++key) {
            mKeyStatus[key] = GLFW_RELEASE;
        }
    }
}