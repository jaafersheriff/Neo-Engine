#include "Renderer/GLSL.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_impl_glfw_gl3.h"

#include "Scene/Scene.hpp"
bool Window::s_cursorEnabled = true;
bool Window::s_imGuiEnabled = false;

int Display::init() {
    /* Init ImGui */
    ImGui_ImplGlfwGL3_Init(s_window, false);
}

void Window::update() {
        /* Update mouse */
        if (!cursorEnabled) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            Mouse::update(x, y);
        }
        /* Update ImGui */
        if (isImGuiEnabled()) {
            ImGui_ImplGlfwGL3_NewFrame(s_cursorEnabled);
        }
}

void Window::toggleImGui() {
    s_imGuiEnabled = !s_imGuiEnabled;
    if (!s_imGuiEnabled) {
        setCursorEnabled(false);
    }
}

void Window::setCursorEnabled(bool enabled) {
    s_cursorEnabled = enabled;
    glfwSetInputMode(s_window, GLFW_CURSOR, s_cursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    if (enabled) {
        Mouse::reset();
        Keyboard::reset();
    }
}

void Window::toggleCursorEnabled() {
    setCursorEnabled(!s_cursorEnabled);
}

void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // toggle fullscreen (f11 or alt+enter)
    if ((key == GLFW_KEY_F11 || key == GLFW_KEY_ENTER && mods & GLFW_MOD_ALT) && action == GLFW_PRESS) {
        // currently fullscreen
        if (glfwGetWindowMonitor(window)) {
            Mouse::reset();
        }
        // currently windowed
        else {
            Mouse::reset();
        }
    }

    if (key == GLFW_KEY_GRAVE_ACCENT && mods & GLFW_MOD_CONTROL && action == GLFW_PRESS) {
        toggleImGui();
    }
    else if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS && isImGuiEnabled()) {
        toggleCursorEnabled();
    }
    else if (isImGuiEnabled() && (ImGui::IsWindowFocused() || ImGui::IsMouseHoveringAnyWindow())) {
        ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
    }
    else
    {
        if (!s_cursorEnabled) {
            Keyboard::setKeyStatus(key, action);
            Scene::sendMessage<KeyMessage>(nullptr, key, action, mods);
        }
    }
}

void Window::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if (isImGuiEnabled() && (ImGui::IsWindowFocused() || ImGui::IsMouseHoveringAnyWindow()) && s_cursorEnabled) {
        ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);
    }
    else 
    {
        if (!s_cursorEnabled) {
            Mouse::setButtonStatus(button, action);
            Scene::sendMessage<MouseMessage>(nullptr, button, action, mods);
        }
    }
}

void Window::scrollCallback(GLFWwindow * window, double dx, double dy) {
    if (isImGuiEnabled() && (ImGui::IsWindowFocused() || ImGui::IsMouseHoveringAnyWindow()) && s_cursorEnabled) {
        ImGui_ImplGlfwGL3_ScrollCallback(window, dx, dy);
    }
    else
    {
        Mouse::scrollDX += dx;
        Mouse::scrollDY += dy;
        Scene::sendMessage<ScrollMessage>(nullptr, float(dx), float(dy));
    }
}

void Window::characterCallback(GLFWwindow *window, unsigned int c) {
    if (isImGuiEnabled() && (ImGui::IsWindowFocused() || ImGui::IsMouseHoveringAnyWindow()) && s_cursorEnabled) {
        ImGui_ImplGlfwGL3_CharCallback(window, c);
    }
}

void Window::framebufferSizeCallback(GLFWwindow * window, int width, int height) {
    Scene::sendMessage<WindowFrameSizeMessage>(nullptr, s_frameSize);
}

void Window::cursorEnterCallback(GLFWwindow * window, int entered) {
    Mouse::reset();
}

