#include "Keyboard.hpp"

#include "GLFW/glfw3.h"

namespace neo {

    int Keyboard::keyStatus[NUM_KEYS] = { GLFW_RELEASE };

    bool Keyboard::isKeyPressed(int key) {
        return keyStatus[key] >= GLFW_PRESS;
    }

    void Keyboard::setKeyStatus(int key, int action) {
        keyStatus[key] = action;
    }

    void Keyboard::reset() {
        for (int key(0); key < NUM_KEYS; ++key) {
            keyStatus[key] = GLFW_RELEASE;
        }
    }
}