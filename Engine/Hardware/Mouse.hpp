#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "ECS/Messaging/Message.hpp"

namespace neo {

    class Mouse {

    public:
        struct MouseResetMessage : public Message { };
        struct MouseButtonMessage : public Message {
            int mButton;
            int mAction;
			MouseButtonMessage(int button, int action)
                : mButton(button)
                , mAction(action)
            {}
        };
        struct ScrollWheelMessage : public Message {
            double mSpeed;
            ScrollWheelMessage(double speed)
                : mSpeed(speed)
            {}
        };
        struct MouseMoveMessage : public Message {
            double mX, mY;
            MouseMoveMessage(double x, double y)
                : mX(x)
                , mY(y)
            {}
        };

        // TODO - maybe move this to ctors
        void init();

        glm::vec2 getPos() const;
        glm::vec2 getSpeed() const;
        float getScrollSpeed() const;
        bool isDown(int) const;

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
        double mDZ = 0;

        void _onReset(const MouseResetMessage& msg);
        void _onButton(const MouseButtonMessage& msg);
        void _onMove(const MouseMoveMessage& msg);
        void _onScrollWheel(const ScrollWheelMessage& msg);

    };
}
