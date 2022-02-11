#include "Keyboard.hpp"

#include "GLFW/glfw3.h"

namespace neo {

    bool Keyboard::isKeyPressed(int key) {
        return mKeyStatus[key] >= GLFW_PRESS;
    }

    void Keyboard::setKeyStatus(int key, int action) {
        mKeyStatus[key] = action;
    }

    void Keyboard::reset() {
        for (int key(0); key < NUM_KEYS; ++key) {
            mKeyStatus[key] = GLFW_RELEASE;
        }
    }
}