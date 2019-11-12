#include "Renderer.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Engine.hpp"
#include "Window/Window.hpp"

#include "ext/imgui/imgui_impl_opengl3.h"
#include "ext/microprofile.h"

namespace neo {

    unsigned Renderer::NEO_GL_MAJOR_VERSION = 4;
    unsigned Renderer::NEO_GL_MINOR_VERSION = 3;
    std::string Renderer::NEO_GLSL_VERSION = "#version 430";
    std::string Renderer::APP_SHADER_DIR;
    CameraComponent *Renderer::mDefaultCamera(nullptr);
    Framebuffer *Renderer::mDefaultFBO;
    std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> Renderer::mComputeShaders;
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
        for (auto& shader : mComputeShaders) {
            shader.second->cleanUp();
        }
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
        MICROPROFILE_ENTERI("Renderer", "resetState", MP_AUTO);
        MICROPROFILE_GPU_ENTERI("RendererGPU", "resetState", MP_AUTO);

        CHECK_GL(glClearColor(0.0f, 0.0f, 0.0f, 1.f));

        CHECK_GL(glEnable(GL_DEPTH_TEST));
        CHECK_GL(glDepthFunc(GL_LESS));

        CHECK_GL(glEnable(GL_CULL_FACE));
        CHECK_GL(glCullFace(GL_BACK));

        CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

        CHECK_GL(glEnable(GL_BLEND));
        CHECK_GL(glBlendEquation(GL_FUNC_ADD));
        CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        CHECK_GL(glActiveTexture(GL_TEXTURE0));
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));

        CHECK_GL(glBindVertexArray(0));

        MICROPROFILE_LEAVE();
        MICROPROFILE_GPU_LEAVE();
    }

    void Renderer::render(float dt) {
        MICROPROFILE_SCOPEI("Renderer", "Renderer::render", MP_AUTO);
        MICROPROFILE_SCOPEGPUI("Renderer::render", MP_AUTO);
        resetState();

        /* Get active shaders */
        std::vector<Shader *> activeComputeShaders = _getActiveShaders(mComputeShaders);
        std::vector<Shader *> activePreShaders = _getActiveShaders(mPreProcessShaders);
        std::vector<Shader *> activePostShaders = _getActiveShaders(mPostShaders);

        /* Run compute */
        if (activeComputeShaders.size()) {
            MICROPROFILE_ENTERI("Renderer", "Compute shaders", MP_AUTO);
            MICROPROFILE_GPU_ENTERI("Renderer", "Compute shaders", MP_AUTO);
            for (auto& shader : activeComputeShaders) {
                resetState();
                MICROPROFILE_ENTERI("Compute ShadersGPU", shader->mName.c_str(), MP_AUTO);
                MICROPROFILE_GPU_ENTERI("Compute ShadersGPU", shader->mName.c_str(), MP_AUTO);
                shader->render(*mDefaultCamera);
                MICROPROFILE_LEAVE();
                MICROPROFILE_GPU_LEAVE();
            }
            MICROPROFILE_LEAVE();
            MICROPROFILE_GPU_LEAVE();
        }

        /* Render all preprocesses */
        if (activePreShaders.size()) {
            MICROPROFILE_ENTERI("Renderer", "PreScene shaders", MP_AUTO);
            MICROPROFILE_GPU_ENTERI("RendererGPU", "PreScene shaders", MP_AUTO);
            for (auto & shader : activePreShaders) {
                resetState();
                MICROPROFILE_ENTERI("PreScene shaders", shader->mName.c_str(), MP_AUTO);
                MICROPROFILE_GPU_ENTERI("PreScene shadersGPU", shader->mName.c_str(), MP_AUTO);
                shader->render(*mDefaultCamera);
                MICROPROFILE_LEAVE();
                MICROPROFILE_GPU_LEAVE();
            }
            MICROPROFILE_LEAVE();
            MICROPROFILE_GPU_LEAVE();
        }

        /* Reset default FBO state */
        MICROPROFILE_ENTERI("Renderer", "Reset DefaultFBO", MP_AUTO);
        MICROPROFILE_GPU_ENTERI("RendererGPU", "Reset DefaultFBO", MP_AUTO);
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
        MICROPROFILE_LEAVE();
        MICROPROFILE_GPU_LEAVE();
 
        /* Render all scene shaders */
        renderScene(*mDefaultCamera);

        /* Post process with ping & pong */
        if (activePostShaders.size()) {
            MICROPROFILE_ENTERI("Renderer", "PostProcess shaders", MP_AUTO);
            MICROPROFILE_GPU_ENTERI("RendererGPU", "PostProcess shaders", MP_AUTO);
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
            MICROPROFILE_LEAVE();
            MICROPROFILE_GPU_LEAVE();
        }

        /* Render imgui */
        if (Engine::mImGuiEnabled) {
            MICROPROFILE_ENTERI("Renderer", "ImGui::render", MP_AUTO);
            MICROPROFILE_GPU_ENTERI("RendererGPU", "ImGui::render", MP_AUTO);
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            MICROPROFILE_LEAVE();
            MICROPROFILE_GPU_LEAVE();
        }

        MICROPROFILE_ENTERI("Renderer", "glfw::swapBuffers", MP_AUTO);
        MICROPROFILE_GPU_ENTERI("Renderer", "glfw::swapBuffers", MP_AUTO);
        glfwSwapBuffers(Window::getWindow());
        MICROPROFILE_LEAVE();
        MICROPROFILE_GPU_LEAVE();
    }

    void Renderer::_renderPostProcess(Shader &shader, Framebuffer *input, Framebuffer *output) {
        MICROPROFILE_ENTERI("PostProcess Shaders", "_renderPostProcess", MP_AUTO);
        MICROPROFILE_GPU_ENTERI("PostProcess ShadersGPU", "_renderPostProcess", MP_AUTO);
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

        // Bind input fbo texture
        input->mTextures[0]->bind();
        shader.loadUniform("inputFBO", input->mTextures[0]->mTextureID);
        input->mTextures[1]->bind();
        shader.loadUniform("inputDepth", input->mTextures[1]->mTextureID);

        // Allow shader to do any prep (eg. bind uniforms) 
        // Also allows shader to override output render target (user responsible for handling)
        MICROPROFILE_ENTERI("PostProcess Shaders", shader.mName.c_str(), MP_AUTO);
        MICROPROFILE_GPU_ENTERI("PostProcess ShadersGPU", shader.mName.c_str(), MP_AUTO);
        shader.render(*mDefaultCamera);

        // Render post process effect
        mesh->draw();

        shader.unbind();
        MICROPROFILE_LEAVE();
        MICROPROFILE_GPU_LEAVE();
        MICROPROFILE_LEAVE();
        MICROPROFILE_GPU_LEAVE();
    }

    void Renderer::renderScene(const CameraComponent &camera) {
        MICROPROFILE_ENTERI("Renderer", "renderScene", MP_AUTO);
        MICROPROFILE_GPU_ENTERI("RendererGPU", "renderScene", MP_AUTO);

        for (auto& shader : mSceneShaders) {
            if (shader.second->mActive) {
                resetState();
                MICROPROFILE_ENTERI("Renderer", shader.second->mName.c_str(), MP_AUTO);
                MICROPROFILE_GPU_ENTERI("RendererGPU", shader.second->mName.c_str(), MP_AUTO);
                shader.second->render(camera);
                MICROPROFILE_LEAVE();
                MICROPROFILE_GPU_LEAVE();
            }
        }
        MICROPROFILE_LEAVE();
        MICROPROFILE_GPU_LEAVE();
    }

    void Renderer::setDefaultFBO(const std::string &name) {
        auto fb = Library::getFBO(name);
        NEO_ASSERT(fb, "Attempting to set an invalid FBO");
        mDefaultFBO = fb;
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