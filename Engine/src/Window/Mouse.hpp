#pragma once

#include <GLFW/glfw3.h>

namespace neo {

    class Mouse {

        public:
            /* x-y position and speed */
            static double x, y;
            static double dx, dy;

            /* Update */
            static void update(double, double);

            /* Denotes if mouse buttons are pressed */
            static void setButtonStatus(int, int);
            static bool isDown(int);
            static void reset();
        private:
            static int mouseButtons[GLFW_MOUSE_BUTTON_LAST];
            static bool isReset;
    };
}
