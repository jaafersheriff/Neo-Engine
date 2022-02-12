#include "Renderer.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Shader/LineShader.hpp"

#include "Engine/Engine.hpp"
#include "Hardware/WindowSurface.hpp"

#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"

#include "imgui_impl_opengl3.h"
#include "microprofile.h"

namespace neo {

#define RENDERER_MP_ENTERD(define, group, name) \
    MICROPROFILE_DEFINE(define, group, name, MP_AUTO);\
    MICROPROFILE_ENTER(define);\
    MICROPROFILE_DEFINE_GPU(define, name,  MP_AUTO);\
    MICROPROFILE_GPU_ENTER(define)

#define RENDERER_MP_ENTER(name) \
    MICROPROFILE_ENTERI("Renderer", name, MP_AUTO);\
    MICROPROFILE_GPU_ENTERI("RendererGPU", name, MP_AUTO)

#define RENDERER_MP_LEAVE() \
    MICROPROFILE_LEAVE();\
    MICROPROFILE_GPU_LEAVE()

    unsigned Renderer::NEO_GL_MAJOR_VERSION = 4;
    unsigned Renderer::NEO_GL_MINOR_VERSION = 4;
    std::string Renderer::NEO_GLSL_VERSION = "#version 440";
    glm::ivec3 Renderer::NEO_MAX_COMPUTE_GROUP_SIZE = glm::ivec3(1);

    std::string Renderer::APP_SHADER_DIR;
    std::string Renderer::ENGINE_SHADER_DIR = "../../Engine/shaders/";
    Framebuffer *Renderer::mDefaultFBO;
    std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> Renderer::mComputeShaders;
    std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> Renderer::mPreProcessShaders;
    std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> Renderer::mSceneShaders;
    std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> Renderer::mPostShaders;
    glm::vec3 Renderer::mClearColor;

    void Renderer::init(const std::string &dir, glm::vec3 clearColor) {
        APP_SHADER_DIR = dir;
        mClearColor = clearColor;

        /* Init default FBO */
        auto backBuffer = Library::createFBO("0");
        backBuffer->mFBOID = 0;
        Renderer::setDefaultFBO("0");

        /* Init default GL state */
        resetState();

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

    void Renderer::render(float dt, WindowSurface& window, ECS& ecs) {
        NEO_UNUSED(dt);
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
                shader->render(ecs);
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
                shader->render(ecs);
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
        glm::ivec2 frameSize = window.getDetails().getSize();
        CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        RENDERER_MP_LEAVE();
 
        /* Render all scene shaders */
        RENDERER_MP_ENTER("renderScene");
        for (auto& shader : mSceneShaders) {
            if (shader.second->mActive) {
                resetState();
                RENDERER_MP_ENTERD(Scene, "Scene Shaders", shader.second->mName.c_str());
                shader.second->render(ecs);
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

            _renderPostProcess(*activePostShaders[0], inputFBO, outputFBO, window.getDetails().getSize(), ecs);

            /* [2, n-1] shaders use ping & pong */
            inputFBO = Library::getFBO("pong");
            outputFBO = Library::getFBO("ping");
            for (unsigned i = 1; i < activePostShaders.size() - 1; i++) {
                _renderPostProcess(*activePostShaders[i], inputFBO, outputFBO, window.getDetails().getSize(), ecs);

                /* Swap ping & pong */
                Framebuffer *temp = inputFBO;
                inputFBO = outputFBO;
                outputFBO = temp;
            }

            /* nth shader writes out to FBO 0 if it hasn't already been done */
            if (activePostShaders.size() > 1) {
                _renderPostProcess(*activePostShaders.back(), inputFBO, Library::getFBO("0"), window.getDetails().getSize(), ecs);
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
        glfwSwapBuffers(window.getWindow());
        RENDERER_MP_LEAVE();
    }

    void Renderer::_renderPostProcess(Shader &shader, Framebuffer *input, Framebuffer *output, glm::ivec2 frameSize, ECS& ecs) {
        RENDERER_MP_ENTER("_renderPostProcess");

        // Reset output FBO
        output->bind();
        resetState();
        CHECK_GL(glDisable(GL_DEPTH_TEST));
        CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));
        // TODO : messaging to resize fbo
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
        shader.render(ecs);

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

    void Renderer::_imguiEditor(ECS& ecs) {
        static bool _showBB = false;
        if (ImGui::Checkbox("Show bounding boxes", &_showBB)) {
            if (_showBB) {
                getShader<LineShader>().mActive = true;
                for (auto box : ecs.getComponents<BoundingBoxComponent>()) {
                    auto line = box->getGameObject().getComponentByType<LineMeshComponent>();
                    if (!line) {
                        line = &ecs.addComponent<LineMeshComponent>(&box->getGameObject());

                        line->mUseParentSpatial = true;
                        line->mWriteDepth = true;
                        line->mOverrideColor = util::genRandomVec3(0.3f, 1.f);

                        glm::vec3 NearLeftBottom{ box->mMin };
                        glm::vec3 NearLeftTop{ box->mMin.x, box->mMax.y, box->mMin.z };
                        glm::vec3 NearRightBottom{ box->mMax.x, box->mMin.y, box->mMin.z };
                        glm::vec3 NearRightTop{ box->mMax.x, box->mMax.y, box->mMin.z };
                        glm::vec3 FarLeftBottom{ box->mMin.x, box->mMin.y,  box->mMax.z };
                        glm::vec3 FarLeftTop{ box->mMin.x, box->mMax.y,     box->mMax.z };
                        glm::vec3 FarRightBottom{ box->mMax.x, box->mMin.y, box->mMax.z };
                        glm::vec3 FarRightTop{ box->mMax };

                        line->addNode(NearLeftBottom);
                        line->addNode(NearLeftTop);
                        line->addNode(NearRightTop);
                        line->addNode(NearRightBottom);
                        line->addNode(NearLeftBottom);
                        line->addNode(FarLeftBottom);
                        line->addNode(FarLeftTop);
                        line->addNode(NearLeftTop);
                        line->addNode(FarLeftTop);
                        line->addNode(FarRightTop);
                        line->addNode(NearRightTop);
                        line->addNode(FarRightTop);
                        line->addNode(FarRightBottom);
                        line->addNode(NearRightBottom);
                        line->addNode(FarRightBottom);
                        line->addNode(FarLeftBottom);
                    }
                }
            }
            else {
                for (auto box : ecs.getComponents<BoundingBoxComponent>()) {
                    auto line = box->getGameObject().getComponentByType<LineMeshComponent>();
                    if (line) {
                        ecs.removeComponent(*line);
                    }
                }
            }
        }
        if (ImGui::TreeNodeEx("Shaders", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto shadersFunc = [&](std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>>& shaders, const std::string swapName) {
                for (unsigned i = 0; i < shaders.size(); i++) {
                    auto& shader = shaders[i];
                    ImGui::PushID(i);
                    bool treeActive = ImGui::TreeNodeEx(shader.second->mName.c_str());
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                        ImGui::SetDragDropPayload(swapName.c_str(), &i, sizeof(unsigned));
                        ImGui::Text("Swap %s", shader.second->mName.c_str());
                        ImGui::EndDragDropSource();
                    }
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload(swapName.c_str())) {
                            IM_ASSERT(payLoad->DataSize == sizeof(unsigned));
                            unsigned payload_n = *(const unsigned*)payLoad->Data;
                            shaders[i].swap(shaders[payload_n]);
                        }
                        ImGui::EndDragDropTarget();
                    }

                    if (treeActive) {
                        ImGui::Checkbox("Active", &shader.second->mActive);
                        ImGui::SameLine();
                        if (ImGui::Button("Reload")) {
                            shader.second->reload();
                        }
                        shader.second->imguiEditor();
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
            };

            if (Renderer::mComputeShaders.size() && ImGui::TreeNodeEx("Compute", ImGuiTreeNodeFlags_DefaultOpen)) {
                shadersFunc(Renderer::mComputeShaders, "COMPUTE_SWAP");
                ImGui::TreePop();
            }
            if (Renderer::mPreProcessShaders.size() && ImGui::TreeNodeEx("Pre process", ImGuiTreeNodeFlags_DefaultOpen)) {
                shadersFunc(Renderer::mPreProcessShaders, "PRESHADER_SWAP");
                ImGui::TreePop();
            }
            if (Renderer::mSceneShaders.size() && ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
                shadersFunc(Renderer::mSceneShaders, "SCENESHADER_SWAP");
                ImGui::TreePop();
            }
            if (Renderer::mPostShaders.size() && ImGui::TreeNodeEx("Post process", ImGuiTreeNodeFlags_DefaultOpen)) {
                shadersFunc(Renderer::mPostShaders, "POSTSHADER_SWAP");
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

    }
}
