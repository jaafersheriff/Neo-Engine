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
        struct WindowDetails {
            glm::ivec2 mFrameSize = {};
            glm::ivec2 mFullscreenSize = {};
            glm::ivec2 mWindowSize = { 1920, 1080 };
            glm::ivec2 mWindowPos = { 0,0 };
            bool mFullscreen = false;
            bool mVSyncEnabled = true;

            glm::ivec2 getSize() const { return mFullscreen ? mFullscreenSize : mWindowSize; }
        };

        int init(const std::string&);
        void update();
        void shutDown();

        int shouldClose() const;
        WindowDetails getDetails() const { return mDetails; }
        float getAspectRatio() const { return mDetails.mFrameSize.x / (float)mDetails.mFrameSize.y; }

        GLFWwindow* getWindow() { return mWindow; }
        void toggleVSync();
        void setSize(const glm::ivec2&);
        void setWindowTitle(const std::string&);

    private:
        GLFWwindow* mWindow = nullptr;
        WindowDetails mDetails;
    };
}
