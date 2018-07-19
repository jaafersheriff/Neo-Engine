#include "Mouse.hpp"

namespace neo {

    double Mouse::x = 0.0;
    double Mouse::y = 0.0;
    double Mouse::dx = 0.0;
    double Mouse::dy = 0.0;
    int Mouse::mouseButtons[GLFW_MOUSE_BUTTON_LAST] = { GLFW_RELEASE };
    bool Mouse::isReset = true;

    void Mouse::update(double newX, double newY) {
        if (isReset) {
            x = newX;
            y = newY;
            isReset = false;
        }

        /* Calculate x-y speed */
        dx = newX - x;
        dy = newY - y;

        /* Set new positions */
        x = newX;
        y = newY;

        // TODO : dw = scroll whell
    }

    bool Mouse::isDown(int button) {
        return mouseButtons[button] >= GLFW_PRESS;
    }

    void Mouse::setButtonStatus(int button, int action) {
        mouseButtons[button] = action;
    }

    void Mouse::reset() {
        isReset = true;
        dx = dy = 0;
    }
}