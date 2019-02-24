#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

namespace neo {

    class Mouse {

        public:
            /* Update */
            static void update(double, double);

            static glm::vec2 getPos();
            static glm::vec2 getSpeed();

            /* Denotes if mouse buttons are pressed */
            static void setButtonStatus(int, int);
            static bool isDown(int);
            static void reset();
        private:
            static int mMouseButtons[GLFW_MOUSE_BUTTON_LAST];
            static bool mIsReset;

            /* x-y position and speed */
            static double mX, mY;
            static double mDX, mDY;


    };
}
