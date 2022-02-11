#pragma once

#define NUM_KEYS 1024

#include "GLFW/glfw3.h"
#include "Messaging/Message.hpp"

namespace neo {

    class Keyboard {
    public:
        struct ResetKeyboardMessage : public Message {
            ResetKeyboardMessage()
            {}
        };
        struct KeyPressedMessage : public Message {
            const int mKey;
            const int mAction;
            KeyPressedMessage(int key, int action)
                : mKey(key)
                , mAction(action)
            {}
        };

        void init();

        bool isKeyPressed(int) const;

        void setKeyStatus(int, int);
        void reset();

    private:
        int mKeyStatus[NUM_KEYS] = { GLFW_RELEASE };
    };
}
