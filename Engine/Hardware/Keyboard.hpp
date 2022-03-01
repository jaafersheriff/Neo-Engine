#pragma once

#define NUM_KEYS GLFW_KEY_LAST

#include "GLFW/glfw3.h"
#include "Messaging/Message.hpp"

namespace neo {

    class Keyboard {
    public:
        struct ResetKeyboardMessage : public Message { };
        struct KeyPressedMessage : public Message {
            const int mKey;
            const int mAction;
            KeyPressedMessage(int key, int action)
                : mKey(key)
                , mAction(action)
            {}
        };

        Keyboard() = default;
        ~Keyboard() = default;
        Keyboard(const Keyboard&) = default;
        Keyboard& operator=(const Keyboard&) = default;
        
        void init();

        bool isKeyPressed(int) const;
        void setKeyStatus(int, int);
        void reset();

    private:
        bool mKeyStatus[NUM_KEYS] = { GLFW_RELEASE };
    };
}
