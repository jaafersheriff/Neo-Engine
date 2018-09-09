#include "MasterRenderer.hpp"
#include "GLHelper/GLHelper.hpp"

#include "NeoEngine.hpp"
#include "Messaging/Messenger.hpp"

#include "Component/ModelComponent/RenderableComponent.hpp"
#include "Window/Window.hpp"

#include "ext/imgui/imgui_impl_opengl3.h"

namespace neo {

    std::string MasterRenderer::APP_SHADER_DIR;
    CameraComponent *MasterRenderer::defaultCamera(nullptr);
    Framebuffer *MasterRenderer::defaultFBO;
    std::vector<std::unique_ptr<Shader>> MasterRenderer::preShaders;
    std::vector<std::unique_ptr<Shader>> MasterRenderer::sceneShaders;
    std::vector<std::unique_ptr<Shader>> MasterRenderer::postShaders;
    std::unordered_map<std::type_index, std::unique_ptr<std::vector<RenderableComponent *>>> MasterRenderer::renderables;

    const char * MasterRenderer::POST_PROCESS_VERT_FILE =
        "#version 330 core\n\
        layout (location = 0) in vec3 vertPos;\
        layout (location = 2) in vec2 vertTex;\
        out vec2 fragTex;\
        void main() { gl_Position = vec4(2 * vertPos, 1); fragTex = vertTex; }";

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
        defaultFBO = Loader::getFBO("0");
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
        /* Get active shaders */
        std::vector<Shader *> activePreShaders = getActiveShaders(preShaders);
        std::vector<Shader *> activePostShaders = getActiveShaders(postShaders);

        /* Render all preprocesses */
        for (auto & shader : activePreShaders) {
            shader->render(*defaultCamera);
        }

        /* Reset default FBO state */
        if (activePostShaders.size()) {
            defaultFBO->bind();
            glm::ivec2 frameSize = Window::getFrameSize();
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));
        }
        else if (activePreShaders.size()) {
            Loader::getFBO("0")->bind();
            glm::ivec2 frameSize = Window::getFrameSize();
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));
        }
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
 
        /* Render all scene shaders */
        renderScene(*defaultCamera);

        /* Post process with ping & pong */
        if (activePostShaders.size()) {
            CHECK_GL(glDisable(GL_DEPTH_TEST));

            /* A single post process shader will render directly from default FBO to FBO 0*/
            if (activePostShaders.size() == 1) {
                renderPostProcess(*activePostShaders[0], defaultFBO, Loader::getFBO("0"));
            }
            /* Multiple post process shaders will use ping & pong*/
            else {
                /* First post process shader reads in from default FBO and writes to ping*/
                renderPostProcess(*activePostShaders[0], defaultFBO, Loader::getFBO("ping"));

                /* [1, n-1] shaders iteratively use ping & pong for input and output */
                Framebuffer *inputFBO = Loader::getFBO("ping");
                Framebuffer *outputFBO = Loader::getFBO("pong");
                for (unsigned i = 1; i < activePostShaders.size() - 1; i++) {
                    renderPostProcess(*activePostShaders[i], inputFBO, outputFBO);

                    /* Swap ping & pong */
                    Framebuffer *temp = inputFBO;
                    inputFBO = outputFBO;
                    outputFBO = temp;
                }

                /* nth shader writes out to FBO 0 */
                if (activePostShaders.size() > 1) {
                    renderPostProcess(*activePostShaders[activePostShaders.size() - 1], inputFBO, Loader::getFBO("0"));
                }
            }
            CHECK_GL(glEnable(GL_DEPTH_TEST));
        }

        /* Render imgui */
        if (NeoEngine::imGuiEnabled) {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        glfwSwapBuffers(Window::getWindow());
    }

    void MasterRenderer::renderPostProcess(Shader &shader, Framebuffer *input, Framebuffer *output) {
        // Reset output FBO
        output->bind();
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        // TODO : messaging to resize fbo
        glm::ivec2 frameSize = Window::getFrameSize();
        CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

        // Bind quad 
        shader.bind();
        auto mesh = Loader::getMesh("quad");
        CHECK_GL(glBindVertexArray(mesh->vaoId));

        // Bind input fbo texture
        input->textures[0]->bind();
        shader.loadUniform("inputFBO", input->textures[0]->textureId);
        input->textures[1]->bind();
        shader.loadUniform("inputDepth", input->textures[1]->textureId);

        // Allow shader to do any prep (eg. bind uniforms)
        shader.render(*defaultCamera);

        // Render post process effect
        mesh->draw();

        shader.unbind();
    }

    void MasterRenderer::renderScene(const CameraComponent &camera) {
       for (auto & shader : sceneShaders) {
            if (shader->active) {
                shader->render(camera);
            }
        }
    }

    void MasterRenderer::setDefaultFBO(const std::string &name) {
        auto fb = Loader::getFBO(name);
        if (fb) {
            defaultFBO = fb;
        }
    }

    std::vector<Shader *> MasterRenderer::getActiveShaders(std::vector<std::unique_ptr<Shader>> &shaders) {
        std::vector<Shader *> ret;
        for (auto & s : shaders) {
            if (s->active) {
                ret.emplace_back(s.get());
            }
        }

        return ret;
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