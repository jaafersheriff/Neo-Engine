#include "Window.hpp"
#include "Mouse.hpp"
#include "Keyboard.hpp"

#include "NeoEngine.hpp"

#include "ext/imgui/imgui_impl_glfw_gl3.h"

#include <iostream>

#define DEFAULT_WITH 1280
#define DEFAULT_HEIGHT 720

namespace neo {

    GLFWwindow *Window::window = nullptr;
    glm::ivec2 Window::frameSize;
    glm::ivec2 Window::fullscreenSize;
    glm::ivec2 Window::windowSize(DEFAULT_WITH, DEFAULT_HEIGHT);
    glm::ivec2 Window::windowPos(0, 0);
    bool Window::fullscreen = false;
    bool Window::vSyncEnabled = true;

    void Window::errorCallback(int error, const char *desc) {
        std::cerr << "Error " << error << ": " << desc << std::endl;
    }

    void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        /* Exit */
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
            return;
        }

        /* Toggle fullscreen (f11 or alt+enter) */
        if ((key == GLFW_KEY_F11 || key == GLFW_KEY_ENTER && mods & GLFW_MOD_ALT) && action == GLFW_PRESS) {
            /* If already full screen */
            if (glfwGetWindowMonitor(window)) {
                fullscreen = false;
                glfwSetWindowMonitor(window, nullptr, windowPos.x, windowPos.y, windowSize.x, windowSize.y, GLFW_DONT_CARE);
            }
            /* If windowed */
            else {
                glfwGetWindowPos(window, &windowPos.x, &windowPos.y);
                fullscreen = true;
                GLFWmonitor * monitor(glfwGetPrimaryMonitor());
                const GLFWvidmode * video(glfwGetVideoMode(monitor));
                glfwSetWindowMonitor(window, monitor, 0, 0, video->width, video->height, video->refreshRate);
            }
            return;
        }
        if (key == GLFW_KEY_GRAVE_ACCENT && mods && action == GLFW_PRESS) {
            NeoEngine::toggleImGui();
        }
        else if (NeoEngine::imGuiEnabled && (ImGui::IsWindowFocused() || ImGui::IsMouseHoveringAnyWindow())) {
            ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
        }
        else {
            Keyboard::setKeyStatus(key, action);
        }
    }

    void Window::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
        if (NeoEngine::imGuiEnabled && (ImGui::IsWindowFocused() || ImGui::IsMouseHoveringAnyWindow())) {
            ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);
        }
        else {
            Mouse::setButtonStatus(button, action);
        }
    }

    void Window::scrollCallback(GLFWwindow * window, double dx, double dy) {
        if (NeoEngine::imGuiEnabled && (ImGui::IsWindowFocused() || ImGui::IsMouseHoveringAnyWindow())) {
            ImGui_ImplGlfwGL3_ScrollCallback(window, dx, dy);
        }
   }

    void Window::characterCallback(GLFWwindow *window, unsigned int c) {
        if (NeoEngine::imGuiEnabled && (ImGui::IsWindowFocused() || ImGui::IsMouseHoveringAnyWindow())) {
            ImGui_ImplGlfwGL3_CharCallback(window, c);
        }
    }

    void Window::windowSizeCallback(GLFWwindow * window, int width, int height) {
        if (width == 0 || height == 0) {
            return;
        }

        if (fullscreen) {
            fullscreenSize.x = width;
            fullscreenSize.y = height;
        }
        else {
            windowSize.x = width;
            windowSize.y = height;
        }
    }

    void Window::framebufferSizeCallback(GLFWwindow * window, int width, int height) {
        if (width == 0 || height == 0) {
            return;
        }

        frameSize.x = width; frameSize.y = height;
        /* Set viewport to window size */
        glViewport(0, 0, width, height);
    }

    void Window::cursorEnterCallback(GLFWwindow * window, int entered) {
        Mouse::reset();
    }

    int Window::initGLFW(const std::string &name) {
        /* Set error callback */
        glfwSetErrorCallback(errorCallback);

        /* Init GLFW */
        if (!glfwInit()) {
            std::cerr << "Error initializing GLFW" << std::endl;
            return 1;
        }

        /* Request version 3.3 of OpenGL */
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_AUTO_ICONIFY, false);

        /* Create GLFW window */
        window = glfwCreateWindow(windowSize.x, windowSize.y, name.c_str(), NULL, NULL);
        if (!window) {
            std::cerr << "Failed to create window" << std::endl;
            glfwTerminate();
            return 1;
        }
        glfwMakeContextCurrent(window);

        /* Set callbacks */
        glfwSetKeyCallback(window, keyCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetScrollCallback(window, scrollCallback);
        glfwSetCharCallback(window, characterCallback);
        glfwSetWindowSizeCallback(window, windowSizeCallback);
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
        glfwSetCursorEnterCallback(window, cursorEnterCallback);

        /* Init GLEW */
        glewExperimental = GL_FALSE;
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "OpenGL Error: " << error << std::endl;
            return 1;
        }
        error = glewInit();
        if (error != GLEW_OK) {
            std::cerr << "Failed to init GLEW" << std::endl;
            return 1;
        }
        glGetError();
        glfwSwapInterval(vSyncEnabled);

        glfwGetFramebufferSize(window, &frameSize.x, &frameSize.y);

        /* Init ImGui */
        ImGui_ImplGlfwGL3_Init(Window::getWindow(), false);

        return 0;
    }

    void Window::update() {
        /* Don't update display if window is minimized */
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
            return;
        }

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        Mouse::update(x, y);

        if (NeoEngine::imGuiEnabled) {
            ImGui_ImplGlfwGL3_NewFrame(true);
        }

        glfwPollEvents();
    }

    void Window::setWindowTitle(const std::string &name) {
        glfwSetWindowTitle(window, name.c_str());
    }

    void Window::setSize(const glm::ivec2 & size) {
        if (!fullscreen) {
            windowSize.x = size.x;
            windowSize.y = size.y;
            glfwSetWindowSize(window, windowSize.x, windowSize.y);
        }
    }

    void Window::toggleVSync() {
        vSyncEnabled = !vSyncEnabled;
        glfwSwapInterval(vSyncEnabled);
    }

    int Window::shouldClose() {
        return glfwWindowShouldClose(window);
    }

    void Window::shutDown() {
        /* Clean up GLFW */
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}