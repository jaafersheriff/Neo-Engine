/* Window class
 * Static GLFW wrapper */
#pragma once

#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include <string>

namespace neo {

    class Window {

        public:
            static int initGLFW(const std::string &);
            static void setWindowTitle(const std::string &);
            static void update();
            static int shouldClose();
            static void shutDown();

            static void setSize(const glm::ivec2 &);
            static glm::ivec2 getSize() { return fullscreen ? fullscreenSize : windowSize; }
            static glm::ivec2 getFrameSize() { return frameSize; }
            static float getAspectRatio() { return float(frameSize.x / frameSize.y); }
            static GLFWwindow * getWindow() { return window; }

            static bool isFullscreen() { return fullscreen; }

            static void toggleVSync();
            static bool isVSyncEnabled() { return vSyncEnabled; }

        private:
            /* GLFW Window */
            static GLFWwindow * window;
            static glm::ivec2 frameSize;
            static glm::ivec2 windowSize;
            static glm::ivec2 windowPos;
            static glm::ivec2 fullscreenSize;
            static bool fullscreen;
            static bool vSyncEnabled;

            /* Callbacks */
            static void errorCallback(int, const char *);
            static void keyCallback(GLFWwindow *, int, int, int, int);
            static void mouseButtonCallback(GLFWwindow *, int, int, int);
            static void scrollCallback(GLFWwindow *, double dx, double dy);
            static void characterCallback(GLFWwindow *, unsigned int);
            static void windowSizeCallback(GLFWwindow *, int width, int height);
            static void framebufferSizeCallback(GLFWwindow *, int width, int height);
            static void cursorEnterCallback(GLFWwindow * window, int entered);


    };
}
