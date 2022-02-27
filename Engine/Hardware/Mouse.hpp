#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Messaging/Message.hpp"

namespace neo {

    class Mouse {

    public:
        struct MouseResetMessage : public Message { };
        struct MouseButtonMessage : public Message {
            const int mButton;
            const int mAction;
			MouseButtonMessage(int button, int action)
                : mButton(button)
                , mAction(action)
            {}
        };
        struct ScrollWheelMessage : public Message {
            const double mSpeed;
            ScrollWheelMessage(double speed)
                : mSpeed(speed)
            {}
        };
        struct MouseMoveMessage : public Message {
            const double mX, mY;
            MouseMoveMessage(double x, double y)
                : mX(x)
                , mY(y)
            {}
        };

        Mouse() = default;
        ~Mouse() = default;
        Mouse(const Mouse&) = default;
        Mouse& operator=(const Mouse&) = default;

        void init();

        glm::vec2 getPos() const;
        glm::vec2 getSpeed() const;
        int getScrollSpeed() const;
        bool isDown(int) const;

        void setButtonStatus(int, int);
        void setScroll(double);
        void reset();
    private:
        int mMouseButtons[GLFW_MOUSE_BUTTON_LAST] = { GLFW_RELEASE };
        bool mIsReset = true;

        /* x-y position and speed */
        double mX = 0.0;
        double mY = 0.0;
        double mDX = 0.0;
        double mDY = 0.0;

        /* Scroll wheel
        * -1 if scrolling back
        *  0 if not scrolling
        *  1 if scrolling forward
        */
        int mZ = 0;

    };
}
