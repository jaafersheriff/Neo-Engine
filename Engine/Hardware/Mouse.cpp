#include "Mouse.hpp"

#include "ECS/Messaging/Messenger.hpp"
#include "Util/Util.hpp"

namespace neo {

    void Mouse::init() {
        Messenger::addReceiver<MouseResetMessage, &Mouse::_onReset>(this);
        Messenger::addReceiver<MouseButtonMessage, &Mouse::_onButton>(this);
        Messenger::addReceiver<ScrollWheelMessage, &Mouse::_onScrollWheel>(this);
        Messenger::addReceiver<MouseMoveMessage, &Mouse::_onMove>(this);
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

    void Mouse::_onReset(const MouseResetMessage& msg) {
        NEO_UNUSED(msg);
        mIsReset = true;
        mDX = mDY = 0;
        mDZ = 0;
        for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++) {
            mMouseButtons[i] = { GLFW_RELEASE };
        }
    }

    void Mouse::_onButton(const MouseButtonMessage& msg) {
        mMouseButtons[msg.mButton] = msg.mAction;
    }

    void Mouse::_onScrollWheel(const ScrollWheelMessage& msg) {
        mDZ = static_cast<int>(msg.mSpeed);
    }

    void Mouse::_onMove(const MouseMoveMessage& msg) {
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
    }

}