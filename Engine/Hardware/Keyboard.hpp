#pragma once

#define NUM_KEYS GLFW_KEY_LAST

#include "GLFW/glfw3.h"
#include "Messaging/Message.hpp"

namespace neo {

    class Keyboard {
    public:
        struct ResetKeyboardMessage : public Message { };
        struct KeyPressedMessage : public Message {
            int mKey;
            int mAction;
            KeyPressedMessage(int key, int action)
                : mKey(key)
                , mAction(action)
            {}
        };

        Keyboard() = default;
        ~Keyboard() = default;
        Keyboard(const Keyboard&) = default;
        Keyboard& operator=(const Keyboard&) = default;
        
        // TODO - maybe move this to ctors
        void init();

        bool isKeyPressed(int) const;

    private:
        bool mKeyStatus[NUM_KEYS] = { GLFW_RELEASE };

        void _onReset(const ResetKeyboardMessage& msg);
        void _onKeyPressed(const KeyPressedMessage& msg);
    };
}
