#include "RenderSystem.hpp"
#include "Util/GLHelper.hpp"

#include "NeoEngine.hpp"

#include "Component/ModelComponent/RenderableComponent.hpp"
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

        /* Render all scene shaders */
        renderScene(*defaultCamera);

        /* Render imgui */
        if (NeoEngine::imGuiEnabled) {
            ImGui::Render();
        }

        glfwSwapBuffers(Window::getWindow());
    }

    void RenderSystem::renderScene(const CameraComponent &camera) {
        for (auto & shader : shaders) {
            if (shader.get()->active) {
                shader.get()->render(*this, camera);
            }
        }
    }

    void RenderSystem::attachCompToShader(const std::type_index &typeI, RenderableComponent *rComp) {
        /* Get vector<RenderableComponent *> that matches this shader */
        auto it(renderables.find(typeI));
        if (it == renderables.end()) {
            renderables.emplace(typeI, std::make_unique<std::vector<RenderableComponent *>>());
            it = renderables.find(typeI);
        }

        it->second->emplace_back(rComp);
    }

    void RenderSystem::detachCompFromShader(const std::type_index &typeI, RenderableComponent *rComp) {
        assert(renderables.count(typeI));
        /* Look in active components in reverse order */
        if (renderables.count(typeI)) {
            auto & comps(*renderables.at(typeI));
            for (int i = int(comps.size()) - 1; i >= 0; --i) {
                if (comps[i] == rComp) {
                    comps.erase(comps.begin() + i);
                    break;
                }
            }
        }
    }

}