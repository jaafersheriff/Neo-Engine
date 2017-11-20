/* Mouse class
 * Maintains mouse movement inside GLFW window */
#pragma once
#ifndef _MOUSE_HPP_
#define _MOUSE_HPP_

#include <GLFW/glfw3.h>

class Mouse {
    public:
        /* Reference to GLFW window */
        GLFWwindow *window;

        /* x-y position and speed */
        double xPos, yPos;
        double dx, dy;

        /* Update */
        void update();

        /* Denotes if mouse buttons are pressed */
        bool isButtonPressed(const int);
};

#endif