#include "Mouse.hpp"

#include "ECS/Messaging/Messenger.hpp"
#include "Util/Util.hpp"

namespace neo {

    void Mouse::init() {
        Messenger::addReceiver<MouseResetMessage>([this](const Message& message) {
            NEO_UNUSED(message);
            mIsReset = true;
            mDX = mDY = 0;
            mDZ = 0;
            for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++) {
                mMouseButtons[i] = { GLFW_RELEASE };
            }
            });

        Messenger::addReceiver<MouseButtonMessage>([this](const Message& message) {
            const MouseButtonMessage& msg(static_cast<const MouseButtonMessage&>(message));
            mMouseButtons[msg.mButton] = msg.mAction;
            });

        Messenger::addReceiver<ScrollWheelMessage>([this](const Message& message) {
            const ScrollWheelMessage& msg(static_cast<const ScrollWheelMessage&>(message));
            mDZ = static_cast<int>(msg.mSpeed);
            });

        Messenger::addReceiver<MouseMoveMessage>([this](const Message& message) {
            const MouseMoveMessage& msg(static_cast<const MouseMoveMessage&>(message));
            if (mIsReset) {
                mX = msg.mX;
                mY = msg.mY;
                mIsReset = false;
            }

            /* Calculate x-y speed */
            mDX = msg.mX - mX;
            mDY = msg.mY - mY;

            /* Set new positions */
            mX = msg.mX;
            mY = msg.mY;

            });
    }

    glm::vec2 Mouse::getPos() const {
        return { mX, mY };
    }

    glm::vec2 Mouse::getSpeed() const {
        return { mDX, mDY };
    }

    float Mouse::getScrollSpeed() const {
        return static_cast<float>(mDZ);
    }

    bool Mouse::isDown(int button) const {
        return mMouseButtons[button] >= GLFW_PRESS;
    }
}