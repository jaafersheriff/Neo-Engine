#include "Renderer.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Engine.hpp"
#include "Window/Window.hpp"

#include "ext/imgui/imgui_impl_opengl3.h"

namespace neo {

    std::string Renderer::APP_SHADER_DIR;
    CameraComponent *Renderer::mDefaultCamera(nullptr);
    Framebuffer *Renderer::mDefaultFBO;
    std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> Renderer::mPreProcessShaders;
    std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> Renderer::mSceneShaders;
    std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> Renderer::mPostShaders;
    glm::vec3 Renderer::mClearColor;

    void Renderer::init(const std::string &dir, CameraComponent *cam, glm::vec3 clearColor) {
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
        mDefaultFBO = Library::getFBO("0");
        mDefaultFBO->mFBOID = 0;
    }

    void Renderer::shutDown() {
        for (auto& shader : mPreProcessShaders) {
            shader.second->cleanUp();
        }
        for (auto& shader : mSceneShaders) {
            shader.second->cleanUp();
        }
        for (auto& shader : mPostShaders) {
            shader.second->cleanUp();
        }
    }

    void Renderer::resetState() {
        CHECK_GL(glClearColor(0.0f, 0.0f, 0.0f, 1.f));

        CHECK_GL(glEnable(GL_DEPTH_TEST));
        CHECK_GL(glDepthFunc(GL_LESS));

        CHECK_GL(glEnable(GL_CULL_FACE));
        CHECK_GL(glCullFace(GL_BACK));

        CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

        CHECK_GL(glEnable(GL_BLEND));
        CHECK_GL(glBlendEquation(GL_FUNC_ADD));
        CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        CHECK_GL(glBindVertexArray(0));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        CHECK_GL(glActiveTexture(GL_TEXTURE0));
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));
    }

    void Renderer::render(float dt) {
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
                Library::getFBO("0")->bind();
            }
        }
        glm::ivec2 frameSize = Window::getFrameSize();
        CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
 
        /* Render all scene shaders */
        renderScene(*mDefaultCamera);

        /* Post process with ping & pong */
        if (activePostShaders.size()) {
            /* Render first post process shader into appropriate output buffer */
            Framebuffer *inputFBO = mDefaultFBO;
            Framebuffer *outputFBO = activePostShaders.size() == 1 ? Library::getFBO("0") : Library::getFBO("pong");

            _renderPostProcess(*activePostShaders[0], inputFBO, outputFBO);

            /* [2, n-1] shaders use ping & pong */
            inputFBO = Library::getFBO("pong");
            outputFBO = Library::getFBO("ping");
            for (unsigned i = 1; i < activePostShaders.size() - 1; i++) {
                _renderPostProcess(*activePostShaders[i], inputFBO, outputFBO);

                /* Swap ping & pong */
                Framebuffer *temp = inputFBO;
                inputFBO = outputFBO;
                outputFBO = temp;
            }

            /* nth shader writes out to FBO 0 if it hasn't already been done */
            if (activePostShaders.size() > 1) {
                _renderPostProcess(*activePostShaders.back(), inputFBO, Library::getFBO("0"));
            }
        }

        /* Render imgui */
        if (Engine::mImGuiEnabled) {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        glfwSwapBuffers(Window::getWindow());
    }

    void Renderer::_renderPostProcess(Shader &shader, Framebuffer *input, Framebuffer *output) {
        // Reset output FBO
        output->bind();
        resetState();
        CHECK_GL(glDisable(GL_DEPTH_TEST));
        CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));
        // TODO : messaging to resize fbo
        glm::ivec2 frameSize = Window::getFrameSize();
        CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

        // Bind quad 
        shader.bind();
        auto mesh = Library::getMesh("quad");
        CHECK_GL(glBindVertexArray(mesh->mVAOID));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh->mVertexBufferID));
        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->mElementBufferID));

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

    void Renderer::renderScene(const CameraComponent &camera) {
       for (auto& shader : mSceneShaders) {
            if (shader.second->mActive) {
                resetState();
                shader.second->render(camera);
            }
        }
    }

    void Renderer::setDefaultFBO(const std::string &name) {
        auto fb = Library::getFBO(name);
        if (fb) {
            mDefaultFBO = fb;
        }
    }

    std::vector<Shader *> Renderer::_getActiveShaders(std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> &shaders) {
        std::vector<Shader *> ret;
        for (auto& shader : shaders) {
            if (shader.second->mActive) {
                ret.emplace_back(shader.second.get());
            }
        }

        return ret;
    }
}