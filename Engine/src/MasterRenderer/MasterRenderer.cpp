#include "MasterRenderer.hpp"
#include "GLHelper/GLHelper.hpp"

#include "NeoEngine.hpp"
#include "Messaging/Messenger.hpp"

#include "Component/ModelComponent/RenderableComponent.hpp"
#include "Window/Window.hpp"

namespace neo {

    std::string MasterRenderer::APP_SHADER_DIR;
    CameraComponent *MasterRenderer::defaultCamera(nullptr);
    std::unordered_map<std::string, std::unique_ptr<Framebuffer>> MasterRenderer::framebuffers;
    Framebuffer *MasterRenderer::defaultFBO;
    std::vector<std::unique_ptr<Shader>> MasterRenderer::preShaders;
    std::vector<std::unique_ptr<Shader>> MasterRenderer::sceneShaders;
    std::vector<std::unique_ptr<Shader>> MasterRenderer::postShaders;
    std::unordered_map<std::type_index, std::unique_ptr<std::vector<RenderableComponent *>>> MasterRenderer::renderables;

    void MasterRenderer::init(const std::string &dir, CameraComponent *cam) {
        APP_SHADER_DIR = dir;
        setDefaultCamera(cam);

        /* Init default GL state */
        resetState();

        /* Init GL window */
        CHECK_GL(glViewport(0, 0, Window::getFrameSize().x, Window::getFrameSize().y));
        Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
            const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
            CHECK_GL(glViewport(0, 0, m.frameSize.x, m.frameSize.y));
        });

        /* Init default FBO */
        defaultFBO = getFBO("default");
        defaultFBO->fboId = 0;
    }

    void MasterRenderer::resetState() {
        CHECK_GL(glEnable(GL_DEPTH_TEST));
        CHECK_GL(glEnable(GL_CULL_FACE));
        CHECK_GL(glCullFace(GL_BACK));
        CHECK_GL(glEnable(GL_BLEND));
        CHECK_GL(glBlendEquation(GL_FUNC_ADD));
        CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
    }

    void MasterRenderer::render(float dt) {
        if (preShaders.size()) {
            /* Render all preprocesses */
            for (auto & shader : preShaders) {
                if (shader.get()->active) {
                    shader.get()->render(*defaultCamera);
                }
            }
            /* Reset default FBO */
            defaultFBO->bind();
            glm::ivec2 frameSize = Window::getFrameSize();
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));
        }

        /* Reset state */
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
 
        /* Render all scene shaders */
        renderScene(*defaultCamera);

        // TODO : post process stack

        /* Render imgui */
        if (NeoEngine::imGuiEnabled) {
            ImGui::Render();
        }

        glfwSwapBuffers(Window::getWindow());
    }

    void MasterRenderer::renderScene(const CameraComponent &camera) {
       for (auto & shader : sceneShaders) {
            if (shader.get()->active) {
                shader.get()->render(camera);
            }
        }
    }

    Framebuffer * MasterRenderer::getFBO(const std::string &name) {
        auto fb = findFBO(name);
        if (!fb) {
            framebuffers.emplace(name, std::make_unique<Framebuffer>());
            fb = framebuffers.find(name)->second.get();
        }
        return fb;
    }

    void MasterRenderer::setDefaultFBO(const std::string &name) {
        auto fb = findFBO(name);
        if (fb) {
            defaultFBO = fb;
        }
    }

    Framebuffer * MasterRenderer::findFBO(const std::string &name) {
        auto it = framebuffers.find(name);
        if (it != framebuffers.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    void MasterRenderer::attachCompToShader(const std::type_index &typeI, RenderableComponent *rComp) {
        /* Get vector<RenderableComponent *> that matches this shader */
        auto it(renderables.find(typeI));
        if (it == renderables.end()) {
            renderables.emplace(typeI, std::make_unique<std::vector<RenderableComponent *>>());
            it = renderables.find(typeI);
        }

        it->second->emplace_back(rComp);
    }

    void MasterRenderer::detachCompFromShader(const std::type_index &typeI, RenderableComponent *rComp) {
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