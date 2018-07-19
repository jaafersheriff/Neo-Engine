#include "RenderSystem.hpp"
#include "Shader/GLHelper.hpp"

#include "Window/Window.hpp"

namespace neo {

    void RenderSystem::init() {
        /* Init default GL state */
        CHECK_GL(glEnable(GL_DEPTH_TEST));
        CHECK_GL(glEnable(GL_CULL_FACE));
        CHECK_GL(glCullFace(GL_BACK));
        CHECK_GL(glEnable(GL_BLEND));
        CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        CHECK_GL(glClearColor(0.2f, 0.3f, 0.4f, 1.f));

        /* Init GL window */
        CHECK_GL(glViewport(0, 0, Window::getFrameSize().x, Window::getFrameSize().y));
    }

    void RenderSystem::update(float dt) {
        /* Reset state */
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        glm::ivec2 size = Window::getFrameSize();
        CHECK_GL(glViewport(0, 0, size.x, size.y));

        /* Render all shaders */
        for (auto & shader : shaders) {
            shader.get()->render(dt, camera);
        }
    }

}