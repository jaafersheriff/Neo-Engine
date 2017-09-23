#include "Keyboard.hpp"

#include <iostream>

// Only supports 0-9 and a-z
bool Keyboard::isKeyPressed(const char key) const {
   // Numbers
   if (key >= '0' && key <= '9') { 
      return glfwGetKey(window, GLFW_KEY_0 + key - '0') == GLFW_PRESS;
   }
   // Letters
   else if (key >= 'a' && key <= 'z') {
      return glfwGetKey(window, GLFW_KEY_A + key - 'a') == GLFW_PRESS;
   }
   // Other
   else if (key == ' ') {
      return glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
   }
 
   return false;
}