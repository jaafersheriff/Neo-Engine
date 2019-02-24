#include "Mouse.hpp"

namespace neo {

    double Mouse::mX = 0.0;
    double Mouse::mY = 0.0;
    double Mouse::mDX = 0.0;
    double Mouse::mDY = 0.0;
    int Mouse::mMouseButtons[GLFW_MOUSE_BUTTON_LAST] = { GLFW_RELEASE };
    bool Mouse::mIsReset = true;

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

        // TODO : dw = scroll whell
    }

    glm::vec2 Mouse::getPos() {
        return glm::vec2(mX, mY);
    }

    glm::vec2 Mouse::getSpeed() {
        return glm::vec2(mDX, mDY);
    }

    bool Mouse::isDown(int button) {
        return mMouseButtons[button] >= GLFW_PRESS;
    }

    void Mouse::setButtonStatus(int button, int action) {
        mMouseButtons[button] = action;
    }

    void Mouse::reset() {
        mIsReset = true;
        mDX = mDY = 0;
    }
}