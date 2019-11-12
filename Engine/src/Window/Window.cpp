#include "Window.hpp"
#include "Mouse.hpp"
#include "Keyboard.hpp"

#include "Engine.hpp"
#include "Messaging/Messenger.hpp"

#include "ext/imgui/imgui_impl_glfw.h"
#include "ext/imgui/imgui_impl_opengl3.h"

#include <iostream>

namespace neo {

    GLFWwindow *Window::mWindow = nullptr;
    glm::ivec2 Window::mFrameSize;
    glm::ivec2 Window::mFullscreenSize;
    glm::ivec2 Window::mWindowSize(1920, 1080);
    glm::ivec2 Window::mWindowPos(0, 0);
    bool Window::mFullscreen = false;
    bool Window::mVSyncEnabled = true;

    void Window::_errorCallback(int error, const char *desc) {
        std::cerr << "Error " << error << ": " << desc << std::endl;
    }

    void Window::_keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        /* Exit */
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
            return;
        }

        /* Toggle mFullscreen (f11 or alt+enter) */
        if ((key == GLFW_KEY_F11 || key == GLFW_KEY_ENTER && mods & GLFW_MOD_ALT) && action == GLFW_PRESS) {
            /* If already full screen */
            if (glfwGetWindowMonitor(window)) {
                mFullscreen = false;
                glfwSetWindowMonitor(window, nullptr, mWindowPos.x, mWindowPos.y, mWindowSize.x, mWindowSize.y, GLFW_DONT_CARE);
            }
            /* If windowed */
            else {
                glfwGetWindowPos(window, &mWindowPos.x, &mWindowPos.y);
                mFullscreen = true;
                GLFWmonitor * monitor(glfwGetPrimaryMonitor());
                const GLFWvidmode * video(glfwGetVideoMode(monitor));
                glfwSetWindowMonitor(window, monitor, 0, 0, video->width, video->height, video->refreshRate);
            }
            return;
        }
        if (key == GLFW_KEY_GRAVE_ACCENT && mods && action == GLFW_PRESS) {
            Engine::toggleImGui();
        }
        else if (Engine::mImGuiEnabled && (ImGui::IsWindowFocused() || ImGui::IsMouseHoveringAnyWindow())) {
            Keyboard::reset();
            ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
        }
        else {
            Keyboard::setKeyStatus(key, action);
        }
    }

    void Window::_mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
        if (Engine::mImGuiEnabled && (ImGui::IsWindowFocused() || ImGui::IsMouseHoveringAnyWindow())) {
            ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
        }
        else {
            Mouse::setButtonStatus(button, action);
        }
    }

    void Window::_scrollCallback(GLFWwindow * window, double dx, double dy) {
        if (Engine::mImGuiEnabled && (ImGui::IsWindowFocused() || ImGui::IsMouseHoveringAnyWindow())) {
            ImGui_ImplGlfw_ScrollCallback(window, dx, dy);
        }
   }

    void Window::_characterCallback(GLFWwindow *window, unsigned int c) {
        if (Engine::mImGuiEnabled && (ImGui::IsWindowFocused() || ImGui::IsMouseHoveringAnyWindow())) {
            ImGui_ImplGlfw_CharCallback(window, c);
        }
    }

    void Window::_windowSizeCallback(GLFWwindow * window, int width, int height) {
        if (width == 0 || height == 0) {
            return;
        }

        if (mFullscreen) {
            mFullscreenSize.x = width;
            mFullscreenSize.y = height;
        }
        else {
            mWindowSize.x = width;
            mWindowSize.y = height;
        }
    }

    void Window::_framebufferSizeCallback(GLFWwindow * window, int width, int height) {
        if (width == 0 || height == 0) {
            return;
        }

        mFrameSize.x = width; mFrameSize.y = height;

        // This message is sent for every frame that the frame is being resized..
        // During a quick click-and-drag resize of the window, this can result 
        // in hundreds of messages 
        Messenger::sendMessage<WindowFrameSizeMessage>(nullptr, mFrameSize);
    }

    void Window::_cursorEnterCallback(GLFWwindow * window, int entered) {
        Mouse::reset();
    }

    int Window::initGLFW(const std::string &name) {
        /* Set error callback */
        glfwSetErrorCallback(_errorCallback);

        /* Init GLFW */
        if (!glfwInit()) {
            std::cerr << "Error initializing GLFW" << std::endl;
            return 1;
        }

        /* Request version 3.3 of OpenGL */
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Renderer::NEO_GL_MAJOR_VERSION);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Renderer::NEO_GL_MINOR_VERSION);
        glfwWindowHint(GLFW_AUTO_ICONIFY, false);

        /* Create GLFW window */
        mWindow = glfwCreateWindow(mWindowSize.x, mWindowSize.y, name.c_str(), NULL, NULL);
        if (!mWindow) {
            glfwTerminate();
            std::cin.get();
            NEO_ASSERT(false, "Failed to create window");
        }
        glfwMakeContextCurrent(mWindow);

        /* Set callbacks */
        glfwSetKeyCallback(mWindow, _keyCallback);
        glfwSetMouseButtonCallback(mWindow, _mouseButtonCallback);
        glfwSetScrollCallback(mWindow, _scrollCallback);
        glfwSetCharCallback(mWindow, _characterCallback);
        glfwSetWindowSizeCallback(mWindow, _windowSizeCallback);
        glfwSetFramebufferSizeCallback(mWindow, _framebufferSizeCallback);
        glfwSetCursorEnterCallback(mWindow, _cursorEnterCallback);

        /* Init GLEW */
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "OpenGL Error: " << error << std::endl;
            return 1;
        }
        glewExperimental = GL_FALSE;
        error = glewInit();
        if (error != GLEW_OK) {
            std::cerr << "Failed to init GLEW" << std::endl;
            return 1;
        }
        glGetError();
        glfwSwapInterval(mVSyncEnabled);

        glfwGetFramebufferSize(mWindow, &mFrameSize.x, &mFrameSize.y);

        /* Init ImGui */
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(mWindow, false);
        ImGui_ImplOpenGL3_Init(Renderer::NEO_GLSL_VERSION.c_str());

        return 0;
    }

    void Window::update() {
        MICROPROFILE_SCOPEI("Window", "Window::update", MP_AUTO);
        /* Don't update display if window is minimized */
        if (glfwGetWindowAttrib(mWindow, GLFW_ICONIFIED)) {
            return;
        }

        MICROPROFILE_ENTERI("Window", "Mouse::update", MP_AUTO);
        double x, y;
        glfwGetCursorPos(mWindow, &x, &y);
        Mouse::update(x, y);
        MICROPROFILE_LEAVE();

        if (Engine::mImGuiEnabled) {
            MICROPROFILE_ENTERI("Window", "ImGui::NewFrame", MP_AUTO);
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            MICROPROFILE_LEAVE();
        }

        MICROPROFILE_ENTERI("Window", "glfwPollEvents", MP_AUTO);
        glfwPollEvents();
        MICROPROFILE_LEAVE();
    }

    void Window::setWindowTitle(const std::string &name) {
        glfwSetWindowTitle(mWindow, name.c_str());
    }

    void Window::setSize(const glm::ivec2 & size) {
        if (!mFullscreen) {
            mWindowSize.x = size.x;
            mWindowSize.y = size.y;
            glfwSetWindowSize(mWindow, mWindowSize.x, mWindowSize.y);
        }
    }

    void Window::toggleVSync() {
        mVSyncEnabled = !mVSyncEnabled;
        glfwSwapInterval(mVSyncEnabled);
    }

    int Window::shouldClose() {
        return glfwWindowShouldClose(mWindow);
    }

    void Window::shutDown() {
        /* Clean up ImGui */
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        /* Clean up GLFW */
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }
}