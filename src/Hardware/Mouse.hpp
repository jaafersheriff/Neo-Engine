#pragma once
#ifndef _MOUSE_HPP_
#define _MOUSE_HPP_

#include <GLFW/glfw3.h>

class Mouse {
    public:
        GLFWwindow *window;
        double xPos, yPos;
        double dx, dy;

        void update();
        bool isButtonPressed(const int);
};

#endif