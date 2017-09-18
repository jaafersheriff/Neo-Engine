#include "Mouse.hpp"

void Mouse::update() {
   // Get new x and y positions on screen
   double newX, newY;
   glfwGetCursorPos(window, &newX, &newY);

   // Calculate position difference
   dx = newX - this->xPos;
   dy = newY - this->yPos;

   // Set current positions
   // TODO: if newX > 0 and newY > 0
   this->xPos = newX;
   this->yPos = newY;

   // TODO : dw = scroll whell
}

bool Mouse::isButtonPressed(const int button) {
   return glfwGetMouseButton(window, button) == GLFW_PRESS;
}
