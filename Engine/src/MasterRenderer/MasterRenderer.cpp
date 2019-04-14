#include "MasterRenderer.hpp"
#include "GLHelper/GLHelper.hpp"

#include "NeoEngine.hpp"

#include "Window/Window.hpp"

#include "ext/imgui/imgui_impl_opengl3.h"

namespace neo {

    std::string MasterRenderer::APP_SHADER_DIR;
    CameraComponent *MasterRenderer::mDefaultCamera(nullptr);
    Framebuffer *MasterRenderer::mDefaultFBO;
    std::vector<std::unique_ptr<Shader>> MasterRenderer::mPreProcessShaders;
    std::vector<std::unique_ptr<Shader>> MasterRenderer::mSceneShaders;
    std::vector<std::unique_ptr<Shader>> MasterRenderer::mPostShaders;
    glm::vec3 MasterRenderer::mClearColor;

    void MasterRenderer::init(const std::string &dir, CameraComponent *cam, glm::vec3 clearColor) {
        APP_SHADER_DIR = dir;
        setDefaultCamera(cam);
        mClearColor = clearColor;

        /* Init default GL state */
        resetState();

        /* Init GL window */
        CHECK_GL(glViewport(0, 0, Window::getFrameSize().x, Window::getFrameSize().y));
        Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
            const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
            CHECK_GL(glViewport(0, 0, m.frameSize.x, m.frameSize.y));
        });

        /* Init default FBO */
        mDefaultFBO = Loader::getFBO("0");
        mDefaultFBO->mFBOID = 0;
    }

    void MasterRenderer::resetState() {
        CHECK_GL(glEnable(GL_DEPTH_TEST));
        CHECK_GL(glEnable(GL_CULL_FACE));
        CHECK_GL(glCullFace(GL_BACK));
        CHECK_GL(glEnable(GL_BLEND));
        CHECK_GL(glBlendEquation(GL_FUNC_ADD));
        CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        CHECK_GL(glClearColor(0.0f, 0.0f, 0.0f, 1.f));
    }

    void MasterRenderer::render(float dt) {
        resetState();

        /* Get active shaders */
        std::vector<Shader *> activePreShaders = _getActiveShaders(mPreProcessShaders);
        std::vector<Shader *> activePostShaders = _getActiveShaders(mPostShaders);

        /* Render all preprocesses */
        for (auto & shader : activePreShaders) {
            resetState();
            shader->render(*mDefaultCamera);
        }

        /* Reset default FBO state */
        if (activePostShaders.size()) {
            mDefaultFBO->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
        }
        else {
            CHECK_GL(glClearColor(mClearColor.x, mClearColor.y, mClearColor.z, 1.f));
            if (activePreShaders.size()) {
                Loader::getFBO("0")->bind();
            }
        }
        glm::ivec2 frameSize = Window::getFrameSize();
        CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
 
        /* Render all scene shaders */
        renderScene(*mDefaultCamera);

        /* Post process with ping & pong */
        if (activePostShaders.size()) {
            CHECK_GL(glDisable(GL_DEPTH_TEST));

            /* Render first post process shader into appropriate output buffer */
            Framebuffer *inputFBO = mDefaultFBO;
            Framebuffer *outputFBO = activePostShaders.size() == 1 ? Loader::getFBO("0") : Loader::getFBO("pong");

            _renderPostProcess(*activePostShaders[0], inputFBO, outputFBO);

            /* [2, n-1] shaders use ping & pong */
            inputFBO = Loader::getFBO("pong");
            outputFBO = Loader::getFBO("ping");
            for (unsigned i = 1; i < activePostShaders.size() - 1; i++) {
                _renderPostProcess(*activePostShaders[i], inputFBO, outputFBO);

                /* Swap ping & pong */
                Framebuffer *temp = inputFBO;
                inputFBO = outputFBO;
                outputFBO = temp;
            }

            /* nth shader writes out to FBO 0 if it hasn't already been done */
            if (activePostShaders.size() > 1) {
                _renderPostProcess(*activePostShaders.back(), inputFBO, Loader::getFBO("0"));
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

    void MasterRenderer::_renderPostProcess(Shader &shader, Framebuffer *input, Framebuffer *output) {
        // Reset output FBO
        output->bind();
        CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));
        // TODO : messaging to resize fbo
        glm::ivec2 frameSize = Window::getFrameSize();
        CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

        // Bind quad 
        shader.bind();
        auto mesh = Loader::getMesh("quad");
        CHECK_GL(glBindVertexArray(mesh->mVAOID));

        // Bind input fbo texture
        input->mTextures[0]->bind();
        shader.loadUniform("inputFBO", input->mTextures[0]->mTextureID);
        input->mTextures[1]->bind();
        shader.loadUniform("inputDepth", input->mTextures[1]->mTextureID);

        // Allow shader to do any prep (eg. bind uniforms) 
        // Also allows shader to override output render target (user responsible for handling)
        shader.render(*mDefaultCamera);

        // Render post process effect
        mesh->draw();

        shader.unbind();
    }

    void MasterRenderer::renderScene(const CameraComponent &camera) {
       for (auto & shader : mSceneShaders) {
            if (shader->mActive) {
                resetState();
                shader->render(camera);
            }
        }
    }

    void MasterRenderer::setDefaultFBO(const std::string &name) {
        auto fb = Loader::getFBO(name);
        if (fb) {
            mDefaultFBO = fb;
        }
    }

    std::vector<Shader *> MasterRenderer::_getActiveShaders(std::vector<std::unique_ptr<Shader>> &shaders) {
        std::vector<Shader *> ret;
        for (auto & s : shaders) {
            if (s->mActive) {
                ret.emplace_back(s.get());
            }
        }

        return ret;
    }
}