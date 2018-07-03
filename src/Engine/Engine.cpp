#include "Engine.hpp"

namespace neo {
    void Engine::run() {
        Window::initGLFW("Name");
        while (!Window::shouldClose()) {
            Window::update(1.f);
        }
    }
}