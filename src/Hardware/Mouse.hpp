#pragma once
#ifndef _MOUSE_HPP_
#define _MOUSE_HPP_

#include <GLFW/glfw3.h>

class Mouse {
   const int MOUSE_LEFT = GLFW_MOUSE_BUTTON_LEFT;
   const int MOUSE_RIGHT = GLFW_MOUSE_BUTTON_RIGHT;

   public:
      double xPos, yPos;
      double dx, dy;

      void update(GLFWwindow *);
      bool isButtonPressed(GLFWwindow *window, const int);
};

#endif