#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

namespace neo {

    class Mouse {

        public:
            /* Update */
            void update(double, double);

            glm::vec2 getPos();
            glm::vec2 getSpeed();
            int getScrollSpeed();

            /* Denotes if mouse buttons are pressed */
            void setButtonStatus(int, int);
            void setScroll(double);
            bool isDown(int);
            void reset();
        private:
            int mMouseButtons[GLFW_MOUSE_BUTTON_LAST] = { GLFW_RELEASE };
            bool mIsReset = true;

            /* x-y position and speed */
            double mX = 0.0;
            double mY = 0.0;
            double mDX = 0.0;
            double mDY = 0.0;

            /* Scroll wheel 
            * -1 if scrolling back
            *  0 if not scrolling
            *  1 if scrolling forward
            */
            int mZ = 0;

    };
}
