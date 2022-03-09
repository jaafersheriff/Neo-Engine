#pragma once

#include "Util/Log/ImGuiConsole.hpp"
#include "Util/Log/Log.hpp"

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <vector>

struct GLFWwindow;

namespace neo {

    class ImGuiManager {
    public:
        struct Viewport {
            bool mIsFocused = false;
            bool mIsHovered = false;
            glm::uvec2 mOffset = { 0,0 };
            glm::uvec2 mSize = { 0,0 };
        };

        static void init(GLFWwindow* window);
        static void update();
        static void render();
        static void reset();
        static void destroy();

        static void begin();
        static void end();

        static void log(const char* log, util::LogSeverity severity);
        static void imGuiEditor();

        static void updateMouse(GLFWwindow* window, int button, int action, int mods);
        static void updateKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void updateCharacter(GLFWwindow* window, unsigned int c);
        static void updateScroll(GLFWwindow* window, double dx, double dy);

        static void toggleImGui();
        static bool isEnabled() { return mIsEnabled; }

        static void updateViewport();
        static bool isViewportFocused();
        static bool isViewportHovered();
        static glm::uvec2 getViewportOffset();
        static glm::uvec2 getViewportSize();
    private:
        static bool mIsEnabled;
        static Viewport mViewport;
        static ImGuiConsole mConsole;
    };
}