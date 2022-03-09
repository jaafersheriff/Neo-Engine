#include "Keyboard.hpp"

#include "GLFW/glfw3.h"
#include "ECS/Messaging/Messenger.hpp"
#include "Util/Util.hpp"

namespace neo {

    void Keyboard::init() {
        Messenger::addReceiver<ResetKeyboardMessage>(nullptr, [this](const neo::Message& msg, ECS& ecs) {
            NEO_UNUSED(ecs);
            const ResetKeyboardMessage& m(static_cast<const ResetKeyboardMessage&>(msg));
            NEO_UNUSED(m);

            reset();
        });

        Messenger::addReceiver<KeyPressedMessage>(nullptr, [this](const neo::Message& msg, ECS& ecs) {
            NEO_UNUSED(ecs);
            const KeyPressedMessage& m(static_cast<const KeyPressedMessage&>(msg));

            setKeyStatus(m.mKey, m.mAction);
        });
    }

    bool Keyboard::isKeyPressed(int key) const {
        NEO_ASSERT(key < NUM_KEYS, "Invalid key");
        return mKeyStatus[key];
    }

    void Keyboard::setKeyStatus(int key, int action) {
        NEO_ASSERT(key < NUM_KEYS, "Invalid key");
        mKeyStatus[key] = action >= GLFW_PRESS;
    }

    void Keyboard::reset() {
        for (int key(0); key < NUM_KEYS; ++key) {
            mKeyStatus[key] = false;
        }
    }
}