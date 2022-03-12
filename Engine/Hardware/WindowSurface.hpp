#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "WindowDetails.hpp"
#include "ECS/Messaging/Message.hpp"

#include <string>

namespace neo {

    class WindowSurface {

        struct ToggleFullscreenMessage : public Message {
            bool mAlreadyFullscreen = false;
            ToggleFullscreenMessage(bool alreadyFullscreen) :
                mAlreadyFullscreen(alreadyFullscreen)
            {}
        };

    public:
        WindowSurface() = default;
        ~WindowSurface() = default;
        WindowSurface(const WindowSurface&) = delete;
        WindowSurface& operator=(const WindowSurface&) = delete;

        int init(const std::string&);
        void reset(const std::string&);
        void updateHardware();
        void shutDown();

        int shouldClose() const;
        const WindowDetails& getDetails() const { return mDetails; }
        float getAspectRatio() const { return mDetails.mSize.x / (float)mDetails.mSize.y; }
        int isMinimized() const;

        GLFWwindow* getWindow() { return mWindow; }
        void toggleVSync();
        void setSize(const glm::ivec2&);

        void onFrameSizeChanged(const FrameSizeMessage& msg);
        void onToggleFullscreen(const ToggleFullscreenMessage& msg);

    private:
        GLFWwindow* mWindow = nullptr;
        WindowDetails mDetails;
    };
}
