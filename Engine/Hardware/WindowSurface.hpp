#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "WindowDetails.hpp"

#include <string>

namespace neo {

    class WindowSurface {
    public:
        WindowSurface() = default;
        ~WindowSurface() = default;
        WindowSurface(const WindowSurface&) = delete;
        WindowSurface& operator=(const WindowSurface&) = delete;

        int init(const std::string&);
        void reset(const std::string&);
        void update();
        void shutDown();

        int shouldClose() const;
        WindowDetails getDetails() const { return mDetails; }
        float getAspectRatio() const { return mDetails.mFrameSize.x / (float)mDetails.mFrameSize.y; }

        GLFWwindow* getWindow() { return mWindow; }
        void toggleVSync();
        void setSize(const glm::ivec2&);

    private:
        GLFWwindow* mWindow = nullptr;
        WindowDetails mDetails;
    };
}
