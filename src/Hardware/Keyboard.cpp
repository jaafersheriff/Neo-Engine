#include "Keyboard.hpp"

#include <iostream>

// Only supports 0-9 and a-z
bool Keyboard::isKeyPressed(const char key) const {
   if (key >= '0' && key <= '9') { 
      return glfwGetKey(window, GLFW_KEY_0 + key - '0') == GLFW_PRESS;
   }
   if (key >= 'a' && key <= 'z') {
      return glfwGetKey(window, GLFW_KEY_A + key - 'a') == GLFW_PRESS;
   }
 
   return false;
}