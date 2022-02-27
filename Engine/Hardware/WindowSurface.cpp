#include "WindowSurface.hpp"
#include "Mouse.hpp"
#include "Keyboard.hpp"

#include "Renderer/Renderer.hpp"
#include "Engine/ImGuiManager.hpp"
#include "Messaging/Messenger.hpp"

#include <iostream>

namespace neo {

    namespace {

        static void _errorCallback(int error, const char* desc) {
            std::cerr << "GLFW Error " << error << ": " << desc << std::endl;
        }

        struct ToggleFullscreenMessage : public Message {
            bool mAlreadyFullscreen = false;
            ToggleFullscreenMessage(bool alreadyFullscreen) :
                mAlreadyFullscreen(alreadyFullscreen)
            {}
        };

    }
    int WindowSurface::init(const std::string& name) {
        /* Set error callback */
        glfwSetErrorCallback(_errorCallback);

        /* Init GLFW */
        if (!glfwInit()) {
            std::cerr << "Error initializing GLFW" << std::endl;
            return 1;
        }

        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_DEPTH_BITS, 32);
        glfwWindowHint(GLFW_STENCIL_BITS, 0);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Renderer::NEO_GL_MAJOR_VERSION);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Renderer::NEO_GL_MINOR_VERSION);
        glfwWindowHint(GLFW_AUTO_ICONIFY, false);
        glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

#ifdef DEBUG_MODE
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        NEO_UNUSED(mode);

        /* Create GLFW window */
        mWindow = glfwCreateWindow(mDetails.mWindowSize.x, mDetails.mWindowSize.y, name.c_str(), NULL, NULL);
        if (!mWindow) {
            glfwTerminate();
            std::cin.get();
            NEO_ASSERT(false, "Failed to create window");
        }
        glfwMakeContextCurrent(mWindow);
        glfwSetWindowUserPointer(mWindow, &mDetails);

        glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            /* Toggle mFullscreen (f11 or alt+enter) */
            if ((key == GLFW_KEY_F11 || key == GLFW_KEY_ENTER && mods & GLFW_MOD_ALT) && action == GLFW_PRESS) {
                Messenger::sendMessage<ToggleFullscreenMessage>(nullptr, glfwGetWindowMonitor(window) != nullptr);
                return;
            }
            if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
                ImGuiManager::toggleImGui();
                WindowDetails& details = *(WindowDetails*)glfwGetWindowUserPointer(window);
                Messenger::sendMessage<WindowFrameSizeMessage>(nullptr, details.mWindowSize);
            }
            if (ImGuiManager::isEnabled() && !ImGuiManager::isViewportFocused()) {
                ImGuiManager::updateKeyboard(window, key, scancode, action, mods);
                Messenger::sendMessage<Keyboard::ResetKeyboardMessage>(nullptr);
            }
            else {
                Messenger::sendMessage<Keyboard::KeyPressedMessage>(nullptr, key, action);
            }
            });
        glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods) {
            bool doImGui = false;
            doImGui |= ImGuiManager::isEnabled() && !ImGuiManager::isViewportHovered();
            doImGui |= ImGuiManager::isEnabled() && ImGuiManager::isViewportHovered() && !ImGuiManager::isViewportFocused() && action == GLFW_PRESS;
            if (doImGui) {
                ImGuiManager::updateMouse(window, button, action, mods);
                Messenger::sendMessage<Mouse::MouseResetMessage>(nullptr);
            }
            else {
                Messenger::sendMessage<Mouse::MouseButtonMessage>(nullptr, button, action);
            }
            });
        glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double dx, double dy) {
            if (ImGuiManager::isEnabled() && !ImGuiManager::isViewportHovered()) {
                ImGuiManager::updateScroll(window, dx, dy);
                Messenger::sendMessage<Mouse::MouseResetMessage>(nullptr);
            }
            else {
                Messenger::sendMessage<Mouse::ScrollWheelMessage>(nullptr, dy);
            }
            });

        glfwSetCharCallback(mWindow, [](GLFWwindow* window, unsigned int c) {
            if (ImGuiManager::isEnabled() && !ImGuiManager::isViewportFocused()) {
                ImGuiManager::updateCharacter(window, c);
            }
            });
        glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
            NEO_UNUSED(window);
            if (width == 0 || height == 0) {
                return;
            }

            Messenger::sendMessage<WindowFrameSizeMessage>(nullptr, glm::uvec2(width, height));
            });

        glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
            NEO_UNUSED(window);
            if (width == 0 || height == 0) {
                return;
            }

            // This message is sent for every frame that the window is being resized..
            // During a quick click-and-drag resize of the window, this can result 
            // in hundreds of messages 
            Messenger::sendMessage<WindowFrameSizeMessage>(nullptr, glm::uvec2(width, height));
            });

        glfwSetCursorEnterCallback(mWindow, [](GLFWwindow* window, int entered) {
            NEO_UNUSED(window, entered);
            Messenger::sendMessage<Mouse::MouseResetMessage>(nullptr);
        });

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
        glfwSwapInterval(mDetails.mVSyncEnabled);

        reset(name);

        return 0;
    }

    void WindowSurface::reset(const std::string& name) {
        glfwSetWindowTitle(mWindow, name.c_str());

        /* Set callbacks */
        Messenger::addReceiver<ToggleFullscreenMessage>(nullptr, [this](const neo::Message& msg, ECS& ecs) {
            NEO_UNUSED(ecs);
            const ToggleFullscreenMessage& m(static_cast<const ToggleFullscreenMessage&>(msg));
            /* If already full screen */
            if (m.mAlreadyFullscreen) {
                mDetails.mFullscreen = false;
                glfwSetWindowMonitor(mWindow, nullptr, mDetails.mWindowPos.x, mDetails.mWindowPos.y, mDetails.mWindowSize.x, mDetails.mWindowSize.y, GLFW_DONT_CARE);
            }
            /* If windowed */
            else {
                glfwGetWindowPos(mWindow, &mDetails.mWindowPos.x, &mDetails.mWindowPos.y);
                mDetails.mFullscreen = true;
                GLFWmonitor* monitor(glfwGetPrimaryMonitor());
                const GLFWvidmode* video(glfwGetVideoMode(monitor));
                glfwSetWindowMonitor(mWindow, monitor, 0, 0, video->width, video->height, video->refreshRate);
            }
            });
        Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [this](const neo::Message& msg, ECS& ecs) {
            NEO_UNUSED(ecs);
            const WindowFrameSizeMessage& m(static_cast<const WindowFrameSizeMessage&>(msg));

            mDetails.mFrameSize.x = m.mFrameSize.x;
            mDetails.mFrameSize.y = m.mFrameSize.y;
            });

        glfwGetFramebufferSize(mWindow, &mDetails.mFrameSize.x, &mDetails.mFrameSize.y);
        Messenger::sendMessage<WindowFrameSizeMessage>(nullptr, mDetails.mFrameSize);
    }

    void WindowSurface::update() {
        MICROPROFILE_SCOPEI("Window", "Window::update", MP_AUTO);

        MICROPROFILE_ENTERI("Window", "Mouse::update", MP_AUTO);
        double x, y;
        glfwGetCursorPos(mWindow, &x, &y);
        Messenger::sendMessage<Mouse::MouseMoveMessage>(nullptr, x, y);
        Messenger::sendMessage<Mouse::ScrollWheelMessage>(nullptr, 0);
        MICROPROFILE_LEAVE();

        MICROPROFILE_ENTERI("Window", "glfwPollEvents", MP_AUTO);
        glfwPollEvents();
        MICROPROFILE_LEAVE();
    }

    void WindowSurface::setSize(const glm::ivec2& size) {
        if (!mDetails.mFullscreen) {
            mDetails.mWindowSize.x = size.x;
            mDetails.mWindowSize.y = size.y;
            glfwSetWindowSize(mWindow, mDetails.mWindowSize.x, mDetails.mWindowSize.y);
        }
    }

    void WindowSurface::toggleVSync() {
        mDetails.mVSyncEnabled = !mDetails.mVSyncEnabled;
        glfwSwapInterval(mDetails.mVSyncEnabled);
    }

    int WindowSurface::shouldClose() const {
        return glfwWindowShouldClose(mWindow);
    }

    int WindowSurface::isMinimized() const {
        return glfwGetWindowAttrib(mWindow, GLFW_ICONIFIED);
    }

    void WindowSurface::shutDown() {
        /* Clean up GLFW */
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }
}