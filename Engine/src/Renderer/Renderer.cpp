#include "Renderer.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Engine.hpp"
#include "Window/Window.hpp"

#include "ext/imgui/imgui_impl_opengl3.h"
#include "ext/microprofile.h"

namespace neo {

#define RENDERER_MP_ENTERD(define, group, name)\
    MICROPROFILE_DEFINE(define, group, name, MP_AUTO);\
    MICROPROFILE_ENTER(define);\
    MICROPROFILE_DEFINE_GPU(define, name,  MP_AUTO);\
    MICROPROFILE_GPU_ENTER(define)

#define RENDERER_MP_ENTER(name)\
    MICROPROFILE_ENTERI("Renderer", name, MP_AUTO);\
    MICROPROFILE_GPU_ENTERI("RendererGPU", name, MP_AUTO)

#define RENDERER_MP_LEAVE()\
    MICROPROFILE_LEAVE();\
    MICROPROFILE_GPU_LEAVE()

    unsigned Renderer::NEO_GL_MAJOR_VERSION = 4;
    unsigned Renderer::NEO_GL_MINOR_VERSION = 3;
    std::string Renderer::NEO_GLSL_VERSION = "#version 430";
    glm::ivec3 Renderer::NEO_MAX_COMPUTE_GROUP_SIZE = glm::ivec3(1);

    std::string Renderer::APP_SHADER_DIR;
    Framebuffer *Renderer::mDefaultFBO;
    std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> Renderer::mComputeShaders;
    std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> Renderer::mPreProcessShaders;
    std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> Renderer::mSceneShaders;
    std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> Renderer::mPostShaders;
    glm::vec3 Renderer::mClearColor;

    void Renderer::init(const std::string &dir, glm::vec3 clearColor) {
        APP_SHADER_DIR = dir;
        mClearColor = clearColor;

        /* Init default GL state */
        resetState();

        /* Init GL window */
        CHECK_GL(glViewport(0, 0, Window::getFrameSize().x, Window::getFrameSize().y));
        Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
            const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
            CHECK_GL(glViewport(0, 0, m.frameSize.x, m.frameSize.y));
        });

        /* Set max work gruop */
        CHECK_GL(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &NEO_MAX_COMPUTE_GROUP_SIZE.x));
        CHECK_GL(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &NEO_MAX_COMPUTE_GROUP_SIZE.y));
        CHECK_GL(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &NEO_MAX_COMPUTE_GROUP_SIZE.z));
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
        RENDERER_MP_ENTER("resetState");

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

        RENDERER_MP_LEAVE();
    }

    void Renderer::render(float dt) {
        RENDERER_MP_ENTER("Renderer::render");

        resetState();

        /* Get active shaders */
        std::vector<Shader *> activeComputeShaders = _getActiveShaders(mComputeShaders);
        std::vector<Shader *> activePreShaders = _getActiveShaders(mPreProcessShaders);
        std::vector<Shader *> activePostShaders = _getActiveShaders(mPostShaders);

        /* Run compute */
        if (activeComputeShaders.size()) {
            RENDERER_MP_ENTER("Compute shaders");

            for (auto& shader : activeComputeShaders) {
                resetState();
                RENDERER_MP_ENTERD(Compute, "Compute shaders", shader->mName.c_str());
                shader->render();
                RENDERER_MP_LEAVE();
            }
            RENDERER_MP_LEAVE();
        }

        /* Render all preprocesses */
        if (activePreShaders.size()) {
            RENDERER_MP_ENTER("PreScene shaders");

            for (auto & shader : activePreShaders) {
                resetState();
                RENDERER_MP_ENTERD(Pre, "PreScene shaders", shader->mName.c_str());
                shader->render();
                RENDERER_MP_LEAVE();
            }
            RENDERER_MP_LEAVE();
        }

        /* Reset default FBO state */
        RENDERER_MP_ENTER("Reset DefaultFBO");
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
        RENDERER_MP_LEAVE();
 
        /* Render all scene shaders */
        RENDERER_MP_ENTER("renderScene");
        for (auto& shader : mSceneShaders) {
            if (shader.second->mActive) {
                resetState();
                RENDERER_MP_ENTERD(Scene, "Scene Shaders", shader.second->mName.c_str());
                shader.second->render();
                RENDERER_MP_LEAVE();
            }
        }
        RENDERER_MP_LEAVE();

        /* Post process with ping & pong */
        if (activePostShaders.size()) {
            RENDERER_MP_ENTER("PostProcess shaders");

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
            RENDERER_MP_LEAVE();
        }

        /* Render imgui */
        if (Engine::mImGuiEnabled) {
            RENDERER_MP_ENTER("ImGui::render");
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            RENDERER_MP_LEAVE();
        }

        RENDERER_MP_LEAVE();

        RENDERER_MP_ENTER("glfwSwapBuffers");
        glfwSwapBuffers(Window::getWindow());
        RENDERER_MP_LEAVE();
    }

    void Renderer::_renderPostProcess(Shader &shader, Framebuffer *input, Framebuffer *output) {
        RENDERER_MP_ENTER("_renderPostProcess");

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
        shader.loadTexture("inputFBO", *input->mTextures[0]); 
        shader.loadTexture("inputDepth", *input->mTextures[1]); 

        RENDERER_MP_LEAVE();

        RENDERER_MP_ENTERD(Post, "PostProcess Shaders", shader.mName.c_str());
        // Allow shader to do any prep (eg. bind uniforms) 
        // Also allows shader to override output render target (user responsible for handling)
        shader.render();

        // Render post process effect
        mesh->draw();

        shader.unbind();
        RENDERER_MP_LEAVE();
    }

    void Renderer::setDefaultFBO(const std::string &name) {
        auto fb = Library::getFBO(name);
        NEO_ASSERT(fb, "Attempting to set an invalid FBO");
        mDefaultFBO = fb;
    }

    std::vector<Shader *> Renderer::_getActiveShaders(std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> &shaders) {
        MICROPROFILE_SCOPEI("Renderer", "_getActiveShaders", MP_AUTO);

        std::vector<Shader *> ret;
        for (auto& shader : shaders) {
            if (shader.second->mActive) {
                ret.emplace_back(shader.second.get());
            }
        }

        return ret;
    }
}
