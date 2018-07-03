#include "Engine.hpp"

namespace neo {

    void Engine::init(const std::string &title, const int width, const int height) {
        Window::initGLFW(title);
        Window::setSize(glm::ivec2(width, height));
    }

    void Engine::run() {
        while (!Window::shouldClose()) {
            Window::update();
        }
    }
}