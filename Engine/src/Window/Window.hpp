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
            static glm::ivec2 getSize() { return mFullscreen ? mFullscreenSize : mWindowSize; }
            static glm::ivec2 getFrameSize() { return mFrameSize; }
            static float getAspectRatio() { return mFrameSize.x / (float)mFrameSize.y; }
            static GLFWwindow * getWindow() { return mWindow; }

            static bool isFullscreen() { return mFullscreen; }

            static void toggleVSync();
            static bool isVSyncEnabled() { return mVSyncEnabled; }

        private:
            /* GLFW Window */
            static GLFWwindow* mWindow;
            static glm::ivec2 mFrameSize;
            static glm::ivec2 mWindowSize;
            static glm::ivec2 mWindowPos;
            static glm::ivec2 mFullscreenSize;
            static bool mFullscreen;
            static bool mVSyncEnabled;

            /* Callbacks */
            static void _errorCallback(int, const char *);
            static void _keyCallback(GLFWwindow *, int, int, int, int);
            static void _mouseButtonCallback(GLFWwindow *, int, int, int);
            static void _scrollCallback(GLFWwindow *, double dx, double dy);
            static void _characterCallback(GLFWwindow *, unsigned int);
            static void _windowSizeCallback(GLFWwindow *, int width, int height);
            static void _framebufferSizeCallback(GLFWwindow *, int width, int height);
            static void _cursorEnterCallback(GLFWwindow * window, int entered);


    };
}
