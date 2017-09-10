#pragma once
#ifndef _KEYBOARD_HPP_
#define _KEYBOARD_HPP_

#include <GLFW/glfw3.h>

class Keyboard {
   public: 
      GLFWwindow *window;
      bool isKeyPressed(const char) const;
};

#endif