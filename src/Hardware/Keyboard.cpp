#include "Keyboard.hpp"

bool Keyboard::isKeyPressed(const char key) {
   // GLFW_keys correspond to ASCII keys
   return glfwGetKey(window, key) == GLFW_PRESS;
}