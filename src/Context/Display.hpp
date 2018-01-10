/* Display class
 * Maintains GLFW window */
#pragma once
#ifndef _DISPLAY_HPP_
#define _DISPLAY_HPP_

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 960

class Display {
    public:
        /* Default window size */
        int width = DEFAULT_WIDTH;
        int height = DEFAULT_HEIGHT;

        /* Reference to GLFW window */
        GLFWwindow *window;

        // TODO : why does display contain reference to projection matrix?
        glm::mat4 projectionMatrix;

        /* Init */
        int init();

        /* Set window title */
        void setTitle(const char *);

        /* Return if window should close */
        int shouldClose();

        /* Update */
        void update();

        /* Swap buffers, poll events */
        void swap();

        /* Shut down */
        void cleanUp();
};

#endif
