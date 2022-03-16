#include "WindowSurface.hpp"
#include "Mouse.hpp"
#include "Keyboard.hpp"

#include "Renderer/Renderer.hpp"
#include "Engine/ImGuiManager.hpp"
#include "ECS/Messaging/Messenger.hpp"

#include "Util/Log/Log.hpp"
#include "Util/ServiceLocator.hpp"
#include <iostream>

namespace neo {

    namespace {

        static void _errorCallback(int error, const char* desc) {
            NEO_LOG_E("GLFW Error %d: %s", error, desc);
        }

    }
    int WindowSurface::init(const std::string& name) {
        /* Set error callback */
        glfwSetErrorCallback(_errorCallback);

        /* Init GLFW */
        NEO_ASSERT(glfwInit(), "Error initializing GLFW");

        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_DEPTH_BITS, 0);
        glfwWindowHint(GLFW_STENCIL_BITS, 0);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ServiceLocator<Renderer>::ref().mDetails.mGLMajorVersion);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, ServiceLocator<Renderer>::ref().mDetails.mGLMinorVersion);
        glfwWindowHint(GLFW_AUTO_ICONIFY, false);
        glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

#ifdef DEBUG_MODE
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        NEO_UNUSED(mode);

        /* Create GLFW window */
        mWindow = glfwCreateWindow(mDetails.mSize.x, mDetails.mSize.y, name.c_str(), NULL, NULL);
        if (!mWindow) {
            glfwTerminate();
            NEO_FAIL("Failed to create window");
        }
        glfwMakeContextCurrent(mWindow);
        glfwSetWindowUserPointer(mWindow, &mDetails);

        glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            /* Toggle mFullscreen (f11 or alt+enter) */
            if ((key == GLFW_KEY_F11 || key == GLFW_KEY_ENTER && mods & GLFW_MOD_ALT) && action == GLFW_PRESS) {
                Messenger::sendMessage<ToggleFullscreenMessage>(glfwGetWindowMonitor(window) != nullptr);
                return;
            }
            if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
                ServiceLocator<ImGuiManager>::ref().toggleImGui();
                if (!ServiceLocator<ImGuiManager>::ref().isEnabled()) {
                    WindowDetails& details = *(WindowDetails*)glfwGetWindowUserPointer(window);
                    int x, y;
                    glfwGetFramebufferSize(window, &x, &y);
                    details.mSize.x = x;
                    details.mSize.y = y;
                    Messenger::sendMessage<FrameSizeMessage>(details.mSize);
                }
            }
            if (ServiceLocator<ImGuiManager>::ref().isEnabled() && !ServiceLocator<ImGuiManager>::ref().isViewportFocused()) {
                ServiceLocator<ImGuiManager>::ref().updateKeyboard(window, key, scancode, action, mods);
                Messenger::sendMessage<Keyboard::ResetKeyboardMessage>();
            }
            else {
                Messenger::sendMessage<Keyboard::KeyPressedMessage>(key, action);
            }
            });
        glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods) {
            bool doImGui = false;
            doImGui |= ServiceLocator<ImGuiManager>::ref().isEnabled() && !ServiceLocator<ImGuiManager>::ref().isViewportHovered();
            doImGui |= ServiceLocator<ImGuiManager>::ref().isEnabled() && ServiceLocator<ImGuiManager>::ref().isViewportHovered() && !ServiceLocator<ImGuiManager>::ref().isViewportFocused() && action == GLFW_PRESS;
            if (doImGui) {
                ServiceLocator<ImGuiManager>::ref().updateMouse(window, button, action, mods);
                Messenger::sendMessage<Mouse::MouseResetMessage>();
            }
            else {
                Messenger::sendMessage<Mouse::MouseButtonMessage>(button, action);
            }
            });
        glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double dx, double dy) {
            if (ServiceLocator<ImGuiManager>::ref().isEnabled() && !ServiceLocator<ImGuiManager>::ref().isViewportHovered()) {
                ServiceLocator<ImGuiManager>::ref().updateScroll(window, dx, dy);
                Messenger::sendMessage<Mouse::MouseResetMessage>();
            }
            else {
                Messenger::sendMessage<Mouse::ScrollWheelMessage>(dy);
            }
            });

        glfwSetCharCallback(mWindow, [](GLFWwindow* window, unsigned int c) {
            if (ServiceLocator<ImGuiManager>::ref().isEnabled() && !ServiceLocator<ImGuiManager>::ref().isViewportFocused()) {
                ServiceLocator<ImGuiManager>::ref().updateCharacter(window, c);
            }
            });

        glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
            NEO_UNUSED(window);
            if (width == 0 || height == 0) {
                return;
            }

            Messenger::sendMessage<FrameSizeMessage>(glm::uvec2(width, height));
            });

        glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
            NEO_UNUSED(window);
            if (width == 0 || height == 0) {
                return;
            }

            Messenger::sendMessage<FrameSizeMessage>(glm::uvec2(width, height));
            });

        glfwSetCursorEnterCallback(mWindow, [](GLFWwindow* window, int entered) {
            NEO_UNUSED(window, entered);
            Messenger::sendMessage<Mouse::MouseResetMessage>();
        });

        /* Init GLEW */
        glewExperimental = GL_FALSE;
        NEO_ASSERT(glewInit()== GLEW_OK, "Failed to init GLEW");

        glfwSwapInterval(mDetails.mVSyncEnabled);

        reset(name);

        return 0;
    }

    void WindowSurface::reset(const std::string& name) {
        glfwSetWindowTitle(mWindow, name.c_str());

        /* Set callbacks */
        Messenger::addReceiver<ToggleFullscreenMessage>([this](const Message& message) {
            const ToggleFullscreenMessage& msg(static_cast<const ToggleFullscreenMessage&>(message));
            /* If already full screen */
            if (msg.mAlreadyFullscreen) {
                mDetails.mFullscreen = false;
                mDetails.mSize = { 1920, 1080 };
                glfwSetWindowMonitor(mWindow, nullptr, mDetails.mPos.x, mDetails.mPos.y, mDetails.mSize.x, mDetails.mSize.y, mDetails.mVSyncEnabled);
            }
            /* If windowed */
            else {
                int x, y;
                glfwGetWindowPos(mWindow, &x, &y);
                mDetails.mPos.x = x;
                mDetails.mPos.y = y;
                mDetails.mFullscreen = true;
                GLFWmonitor* monitor(glfwGetPrimaryMonitor());
                const GLFWvidmode* video(glfwGetVideoMode(monitor));
                glfwSetWindowMonitor(mWindow, monitor, 0, 0, video->width, video->height, video->refreshRate);
            }
        });

        Messenger::addReceiver<FrameSizeMessage>([this](const Message& message) {
            const FrameSizeMessage& msg(static_cast<const FrameSizeMessage&>(message));
            mDetails.mSize.x = msg.mSize.x;
            mDetails.mSize.y = msg.mSize.y;
        });

        if (!ServiceLocator<ImGuiManager>::empty()) {
            if (!ServiceLocator<ImGuiManager>::ref().isEnabled()) {
                int x, y;
                glfwGetFramebufferSize(mWindow, &x, &y);
                mDetails.mSize.x = x;
                mDetails.mSize.y = y;
                Messenger::sendMessage<FrameSizeMessage>(mDetails.mSize);
            }
        }
    }

    void WindowSurface::updateHardware() {
        if (!ServiceLocator<ImGuiManager>::ref().isEnabled()) {
            double x, y;
            glfwGetCursorPos(mWindow, &x, &y);
            y = mDetails.mSize.y - y;
            Messenger::sendMessage<Mouse::MouseMoveMessage>(x, y);
        }
        Messenger::sendMessage<Mouse::ScrollWheelMessage>(0.0);
        MICROPROFILE_ENTERI("Window", "glfwPollEvents", MP_AUTO);
        glfwPollEvents();
        MICROPROFILE_LEAVE();
    }

    void WindowSurface::setSize(const glm::ivec2& size) {
        if (!mDetails.mFullscreen) {
            mDetails.mSize.x = size.x;
            mDetails.mSize.y = size.y;
            glfwSetWindowSize(mWindow, mDetails.mSize.x, mDetails.mSize.y);
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