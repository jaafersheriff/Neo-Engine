#include "Mouse.hpp"
#include <iostream>

void Mouse::update(GLFWwindow *window) {
   // Get new x and y positions on screen
   double newX, newY;
   glfwGetCursorPos(window, &newX, &newY);

   // Calculate d
   dx = newX - this->xPos;
   dy = newX - this->yPos;

   // Set current positions
   this->xPos = newX;
   this->yPos = newY;

   // TODO : dw = scroll whell
}

bool Mouse::isButtonPressed(GLFWwindow* window, const int button) {
   return glfwGetMouseButton(window, button) == GLFW_PRESS;
}
