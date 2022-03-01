#pragma once

#include <imgui/imgui.h>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace neo {
    class ImGuiManager {
    public:

        static void init(GLFWwindow* window);
        static void update();
        static void begin();
        static void end();
        static void render();
        static void destroy();

        static void updateMouse(GLFWwindow* window, int button, int action, int mods);
        static void updateKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void updateCharacter(GLFWwindow* window, unsigned int c);
        static void updateScroll(GLFWwindow* window, double dx, double dy);

        static void toggleImGui() { mIsEnabled = !mIsEnabled; }
        static bool isEnabled() { return mIsEnabled; }

        static void updateViewport();
        static bool isViewportFocused() { return mIsViewportFocused; }
        static bool isViewportHovered() { return mIsViewportHovered; }
        static glm::vec2 getViewportSize() { return mViewportSize; }
    private:
        static bool mIsEnabled;
        static bool mIsViewportFocused;
        static bool mIsViewportHovered;
        static glm::vec2 mViewportOffset;
        static glm::vec2 mViewportSize;
    };
}