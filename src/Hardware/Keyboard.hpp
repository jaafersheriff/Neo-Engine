/* Keyboard class 
 * Contains method to know if a key is pressed */
#pragma once
#ifndef _KEYBOARD_HPP_
#define _KEYBOARD_HPP_

#include <GLFW/glfw3.h>

class Keyboard {
    public: 
        /* Reference to GLFW window */
        GLFWwindow *window;
        
        /* Denotes whether a key is pressed */
        bool isKeyPressed(const char) const;
};

#endif