#include "Mouse.hpp"

#include "Messaging/Messenger.hpp"
#include "Util/Util.hpp"

namespace neo {

    void Mouse::init() {
        Messenger::addReceiver<MouseResetMessage>(nullptr, [this](const neo::Message& msg, ECS& ecs) {
            NEO_UNUSED(msg, ecs);
            reset();
        });

        Messenger::addReceiver<MouseButtonMessage>(nullptr, [this](const neo::Message& msg, ECS& ecs) {
            NEO_UNUSED(ecs);
            const MouseButtonMessage& m(static_cast<const MouseButtonMessage&>(msg));
            setButtonStatus(m.mButton, m.mAction);
        });

        Messenger::addReceiver<ScrollWheelMessage>(nullptr, [this](const neo::Message& msg, ECS& ecs) {
            NEO_UNUSED(ecs);
            const ScrollWheelMessage& m(static_cast<const ScrollWheelMessage&>(msg));
            setScroll(m.mSpeed);
        });

        Messenger::addReceiver<MouseMoveMessage>(nullptr, [this](const neo::Message& msg, ECS& ecs) {
            NEO_UNUSED(ecs);
            const MouseMoveMessage& m(static_cast<const MouseMoveMessage&>(msg));
            if (mIsReset) {
                mX = m.mX;
                mY = m.mY;
                mIsReset = false;
            }

            /* Calculate x-y speed */
            mDX = m.mX - mX;
            mDY = m.mY - mY;

            /* Set new positions */
            mX = m.mX;
            mY = m.mY;
        });
    }

    glm::vec2 Mouse::getPos() const {
        return { mX, mY };
    }

    glm::vec2 Mouse::getSpeed() const {
        return { mDX, mDY };
    }

    int Mouse::getScrollSpeed() const {
        return mZ;
    }

    bool Mouse::isDown(int button) const {
        return mMouseButtons[button] >= GLFW_PRESS;
    }

    void Mouse::setButtonStatus(int button, int action) {
        mMouseButtons[button] = action;
    }

    void Mouse::setScroll(double newScroll) {
        mZ = static_cast<int>(newScroll);
    }

    void Mouse::reset() {
        mIsReset = true;
        mDX = mDY = 0;
        mZ = 0;
        for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++) {
            mMouseButtons[i] = { GLFW_RELEASE };
        }
    }
}