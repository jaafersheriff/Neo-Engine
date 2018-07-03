#include "NeoEngine.hpp"

namespace neo {

    void NeoEngine::init(const std::string &title, const int width, const int height) {
        Window::initGLFW(title);
        Window::setSize(glm::ivec2(width, height));
    }

    void NeoEngine::run() {
        while (!Window::shouldClose()) {
            Window::update();
        }
    }
}