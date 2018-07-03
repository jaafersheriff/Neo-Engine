#include "Window.hpp"

#include <iostream>

#define DEFAULT_WITH 1280
#define DEFAULT_HEIGHT 720

namespace neo {

    GLFWwindow *Window::window = nullptr;
    glm::ivec2 Window::frameSize;
    glm::ivec2 Window::fullscreenSize;
    glm::ivec2 Window::windowSize(DEFAULT_WITH, DEFAULT_HEIGHT);
    bool Window::fullscreen = false;
    bool Window::vSyncEnabled = true;

    void Window::errorCallback(int error, const char *desc) {
             std::cerr << "Error " << error << ": " << desc << std::endl;
    }


}