/* Window class
 * Static GLFW wrapper */
#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include <string>

namespace neo {

    class WindowSurface {

    public:

        int initGLFW(const std::string&);
        void setWindowTitle(const std::string&);
        void update();
        int shouldClose();
        void shutDown();

        void setSize(const glm::ivec2&);
        glm::ivec2 getSize() { return mFullscreen ? mFullscreenSize : mWindowSize; }
        glm::ivec2 getFrameSize() { return mFrameSize; }
        float getAspectRatio() { return mFrameSize.x / (float)mFrameSize.y; }
        GLFWwindow* getWindow() { return mWindow; }

        bool isFullscreen() { return mFullscreen; }

        void toggleVSync();
        bool isVSyncEnabled() { return mVSyncEnabled; }

    private:
        GLFWwindow* mWindow = nullptr;
        glm::ivec2 mFrameSize;
        glm::ivec2 mFullscreenSize;
        glm::ivec2 mWindowSize = { 1920, 1080 };
        glm::ivec2 mWindowPos = { 0,0 };
        bool mFullscreen = false;
        bool mVSyncEnabled = true;

    };
}
