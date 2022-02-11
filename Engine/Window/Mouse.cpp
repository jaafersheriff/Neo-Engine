#include "Mouse.hpp"

namespace neo {

    void Mouse::update(double newX, double newY) {
        if (mIsReset) {
            mX = newX;
            mY = newY;
            mIsReset = false;
        }

        /* Calculate x-y speed */
        mDX = newX - mX;
        mDY = newY - mY;

        /* Set new positions */
        mX = newX;
        mY = newY;
    }

    glm::vec2 Mouse::getPos() {
        return { mX, mY };
    }

    glm::vec2 Mouse::getSpeed() {
        return { mDX, mDY };
    }

    int Mouse::getScrollSpeed() {
        return mZ;
    }

    bool Mouse::isDown(int button) {
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
    }
}