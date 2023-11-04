#include "Renderer/pch.hpp"
#include "Renderer.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"

#include "Messaging/Message.hpp"
#include "Messaging/Messenger.hpp"

#include "Shader/BlitShader.hpp"
#include "Shader/LineShader.hpp"

#include "Engine/Engine.hpp"
#include "Engine/ImGuiManager.hpp"
#include "Hardware/WindowSurface.hpp"

#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"

#include <GLFW/glfw3.h>
#include <imgui_impl_opengl3.h>
#include <tracy/TracyOpenGL.hpp>

namespace neo {

    void OpenGLMessageCallback(
        unsigned source,
        unsigned type,
        unsigned id,
        unsigned severity,
        int length,
        const char* message,
        const void* userParam) {
        NEO_UNUSED(id, length, userParam);

        static std::unordered_map<GLenum, const char*> sSourceString = {
            {GL_DEBUG_SOURCE_API, "API"},
            {GL_DEBUG_SOURCE_WINDOW_SYSTEM, "Window"},
            {GL_DEBUG_SOURCE_SHADER_COMPILER, "Shader Compiler"},
            {GL_DEBUG_SOURCE_THIRD_PARTY, "3rd Party"},
            {GL_DEBUG_SOURCE_APPLICATION, "Application"},
            {GL_DEBUG_SOURCE_OTHER, "Other"},
        };

        static std::unordered_map<GLenum, const char*> sTypeString{
            {GL_DEBUG_TYPE_ERROR, "Error"},
            {GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "Deprecated Behavior"},
            {GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "Undefined Behavior"},
            {GL_DEBUG_TYPE_PORTABILITY, "Portability"},
            {GL_DEBUG_TYPE_PERFORMANCE, "Performance"},
            {GL_DEBUG_TYPE_MARKER, "Marker"},
            {GL_DEBUG_TYPE_PUSH_GROUP, "Push Group"},
            {GL_DEBUG_TYPE_POP_GROUP, "Pop Group"},
            {GL_DEBUG_TYPE_OTHER, "Other"},
        };

        char glBuf[512];
        sprintf(glBuf, "[GL %s] [%s]: %s", sSourceString.at(source), sTypeString.at(type), message);

        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:         NEO_LOG_E(glBuf); return;
            case GL_DEBUG_SEVERITY_MEDIUM:       NEO_LOG_W(glBuf); return;
            case GL_DEBUG_SEVERITY_LOW:          NEO_LOG_W(glBuf); return;
            case GL_DEBUG_SEVERITY_NOTIFICATION: NEO_LOG_I(glBuf); return;
        }

        NEO_FAIL("Unknown severity level!");
    }

    Renderer::Renderer(int GLMajor, int GLMinor) {
        mDetails.mGLMajorVersion = GLMajor;
        mDetails.mGLMinorVersion = GLMinor;
        std::stringstream glsl;
        glsl << "#version " << GLMajor << GLMinor << "0";
        mDetails.mGLSLVersion = glsl.str();
    }

    Renderer::~Renderer() {
    }

    void Renderer::setDemoConfig(IDemo::Config config) {
        mClearColor = config.clearColor;
    }

    void Renderer::init() {
	#ifdef DEBUG_MODE
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);
		
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	#endif
        // if (!mBlitShader) {
        //     mBlitShader = new BlitShader;
        // }

        /* Init default FBO */
        mBackBuffer = Library::createFBO("0");
        mBackBuffer->mFBOID = 0;

        mDefaultFBO = Library::createFBO("backbuffer");
        TextureFormat format = { GL_RGB, GL_RGB, GL_LINEAR, GL_CLAMP_TO_EDGE };
        mDefaultFBO->attachColorTexture({ 1, 1 }, format);
        mDefaultFBO->attachDepthTexture({ 1, 1 }, GL_LINEAR, GL_CLAMP_TO_EDGE);
        mDefaultFBO->initDrawBuffers();
        mDefaultFBO->bind();

        /* Init default GL state */
        resetState();

        Messenger::removeReceiver<FrameSizeMessage>(this);
        Messenger::addReceiver<FrameSizeMessage, &Renderer::_onFrameSizeChanged>(this);
    }

    void Renderer::_onFrameSizeChanged(const FrameSizeMessage& msg) {
        Library::getFBO("backbuffer")->resize(msg.mSize);
        auto ping = Library::getFBO("ping");
        if (ping) {
            ping->resize(msg.mSize);
        }
        auto pong = Library::getFBO("pong");
        if (pong) {
            pong->resize(msg.mSize);
        }
    }

    void Renderer::resetState() {
        TRACY_GPU();

        glClearColor(0.0f, 0.0f, 0.0f, 1.f);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindVertexArray(0);
    }

    void Renderer::render(WindowSurface& window, IDemo* demo, ECS& ecs) {
        mStats = {};

        TRACY_GPU();
        resetState();

            /* Get active shaders */
            // std::vector<Shader*> activeComputeShaders = _getActiveShaders(mComputeShaders);
            // std::vector<Shader*> activePreShaders = _getActiveShaders(mPreProcessShaders);
            // std::vector<Shader*> activePostShaders = _getActiveShaders(mPostShaders);

            /* Run compute */
            // if (activeComputeShaders.size()) {
            //	 TRACY_GPUN("Compute shaders");

            //     for (auto& shader : activeComputeShaders) {
            //         resetState();
            //         mStats.mNumShaders++;
            //         shader->render(ecs);
            //     }
            // }

            /* Render all preprocesses */
            // if (activePreShaders.size()) {
            // 	TRACY_GPUN("PreScene shaders");

            //     for (auto& shader : activePreShaders) {
            //         resetState();
            //         mStats.mNumShaders++;
            //         shader->render(ecs);
            //     }
            // }

        /* Reset default FBO state */
        glm::ivec2 frameSize = window.getDetails().mSize;
        {
            TRACY_GPUN("Reset DefaultFBO");
            mDefaultFBO->bind();
            glClearColor(mClearColor.x, mClearColor.y, mClearColor.z, 1.f);
            glViewport(0, 0, frameSize.x, frameSize.y);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

            /* Render all scene shaders */
            // TRACY_GPUN("renderScene");
            // for (auto& shader : mSceneShaders) {
            //     if (shader.second->mActive) {
            //         resetState();
            //         mStats.mNumShaders++;
            //         shader.second->render(ecs);
            //     }
            // }

            /* Post process with ping & pong */
            // if (activePostShaders.size()) {
            //     RENDERER_MP_ENTER("PostProcess shaders");

            //     /* Render first post process shader into appropriate output buffer */
            //     Framebuffer* inputFBO = mDefaultFBO;
            //     Framebuffer* outputFBO = Library::getFBO("pong");

            //     _renderPostProcess(*activePostShaders[0], inputFBO, outputFBO, window.getDetails().mSize, ecs);

            //     /* [2, n-1] shaders use ping & pong */
            //     inputFBO = Library::getFBO("pong");
            //     outputFBO = Library::getFBO("ping");
            //     for (unsigned i = 1; i < activePostShaders.size() - 1; i++) {
            //         _renderPostProcess(*activePostShaders[i], inputFBO, outputFBO, window.getDetails().mSize, ecs);

            //         /* Swap ping & pong */
            //         Framebuffer* temp = inputFBO;
            //         inputFBO = outputFBO;
            //         outputFBO = temp;
            //     }

            //     /* nth shader writes out to FBO 0 if it hasn't already been done */
            //     _renderPostProcess(*activePostShaders.back(), inputFBO, mDefaultFBO, window.getDetails().mSize, ecs);
            //     RENDERER_MP_LEAVE();
            // }
            demo->render(ecs);

            /* Render imgui */
            if (!ServiceLocator<ImGuiManager>::empty() && ServiceLocator<ImGuiManager>::ref().isEnabled()) {
                TRACY_GPUN("ImGuiManager.render");
                mBackBuffer->bind();
                ServiceLocator<ImGuiManager>::ref().render();
            }
            else {
                // RENDERER_MP_ENTER("Final Blit");
                // if (!mBlitShader) {
                //     mBlitShader = new BlitShader;
                // }
                // mBackBuffer->bind();
                // resetState();
                // glDisable(GL_DEPTH_TEST);
                // glClearColor(0.f, 0.f, 0.f, 1.f);
                // glClear(GL_COLOR_BUFFER_BIT);
                // glViewport(0, 0, frameSize.x, frameSize.y);

                // mBlitShader->bind();
                // auto meshData = Library::getMesh("quad");
                // glBindVertexArray(meshData.mMesh->mVAOID);

                // // Bind input fbo texture
                // mBlitShader->loadTexture("inputTexture", *mDefaultFBO->mTextures[0]);

                // // Render 
                // meshData.mMesh->draw();
                // mBlitShader->unbind();
                // RENDERER_MP_LEAVE();
            }

        /* Post process with ping & pong */
        // if (activePostShaders.size()) {
        //     TRACY_GPUN("PostProcess shaders");

        //     /* Render first post process shader into appropriate output buffer */
        //     Framebuffer* inputFBO = mDefaultFBO;
        //     Framebuffer* outputFBO = activePostShaders.size() == 1 ? Library::getFBO("backbuffer") : Library::getFBO("pong");

        //     _renderPostProcess(*activePostShaders[0], inputFBO, outputFBO, window.getDetails().mSize, ecs);

        //     /* [2, n-1] shaders use ping & pong */
        //     inputFBO = Library::getFBO("pong");
        //     outputFBO = Library::getFBO("ping");
        //     for (unsigned i = 1; i < activePostShaders.size() - 1; i++) {
        //         _renderPostProcess(*activePostShaders[i], inputFBO, outputFBO, window.getDetails().mSize, ecs);

        //         /* Swap ping & pong */
        //         Framebuffer* temp = inputFBO;
        //         inputFBO = outputFBO;
        //         outputFBO = temp;
        //     }

        //     /* nth shader writes out to FBO 0 if it hasn't already been done */
        //     if (activePostShaders.size() > 1) {
        //         _renderPostProcess(*activePostShaders.back(), inputFBO, Library::getFBO("backbuffer"), window.getDetails().mSize, ecs);
        //     }
        // }
        // else if (mDefaultFBO != Library::getFBO("backbuffer")) {
        //     _renderPostProcess(*mBlitShader, mDefaultFBO, Library::getFBO("backbuffer"), window.getDetails().mSize, ecs);
        // }

        // /* Render imgui */
        // if (!ServiceLocator<ImGuiManager>::empty() && ServiceLocator<ImGuiManager>::ref().isEnabled()) {
        //     TRACY_GPUN("ImGuiManager.render");
        //     mBackBuffer->bind();
        //     ServiceLocator<ImGuiManager>::ref().render();
        // }
        // else {
        //     TRACY_GPUN("Final Blit");
        //     _renderPostProcess(*mBlitShader, Library::getFBO("backbuffer"), mBackBuffer, window.getDetails().mSize, ecs);
        // }
    }

    // void Renderer::_renderPostProcess(Shader &shader, Framebuffer *input, Framebuffer *output, glm::ivec2 frameSize, ECS& ecs) {
    //     TRACY_GPU();
    //     mStats.mNumShaders++;

    //     // Reset output FBO
    //     output->bind();
    //     resetState();
    //     glDisable(GL_DEPTH_TEST);
    //     glClearColor(0.f, 0.f, 0.f, 1.f);
    //     glClear(GL_COLOR_BUFFER_BIT);
    //     glViewport(0, 0, frameSize.x, frameSize.y);

    //     // Bind quad 
    //     shader.bind();
    //     auto meshData = Library::getMesh("quad");
    //     glBindVertexArray(meshData.mMesh->mVAOID);

    //     // Bind input fbo texture
    //     shader.loadTexture("inputFBO", *input->mTextures[0]); 
    //     shader.loadTexture("inputDepth", *input->mTextures[1]); 

    //     // Allow shader to do any prep (eg. bind uniforms) 
    //     // Also allows shader to override output render target (user responsible for handling)
    //     shader.render(ecs);

    //     // Render post process effect
    //     meshData.mMesh->draw();

    //     shader.unbind();
    // }

    // std::vector<Shader *> Renderer::_getActiveShaders(std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> &shaders) {
    //     std::vector<Shader *> ret;
    //     for (auto& shader : shaders) {
    //         if (shader.second->mActive) {
    //             ret.emplace_back(shader.second.get());
    //         }
    //     }

    //     return ret;
    // }

    void Renderer::imGuiEditor(WindowSurface& window, ECS& ecs) {
        NEO_UNUSED(ecs);

        ImGui::Begin("Viewport");
        ServiceLocator<ImGuiManager>::ref().updateViewport();
        glm::vec2 viewportSize = ServiceLocator<ImGuiManager>::ref().getViewportSize();
        if (viewportSize.x != 0 && viewportSize.y != 0) {
#pragma warning(push)
#pragma warning(disable: 4312)
            ImGui::Image(reinterpret_cast<ImTextureID>(Library::getFBO("backbuffer")->mTextures[0]->mTextureID), { viewportSize.x, viewportSize.y }, ImVec2(0, 1), ImVec2(1, 0));
#pragma warning(pop)
        }
        ImGui::End();

        ImGui::Begin("Renderer");
        if (ImGui::TreeNodeEx("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextWrapped("Num Draws: %d", mStats.mNumDraws);
            ImGui::TextWrapped("Num Shaders: %d", mStats.mNumShaders);
            ImGui::TextWrapped("Num Triangles: %d", mStats.mNumTriangles);
            ImGui::TextWrapped("Num Uniforms: %d", mStats.mNumUniforms);
            ImGui::TextWrapped("Num Samplers: %d", mStats.mNumSamplers);
            if (auto hardwareDetails = ecs.getSingleView<MouseComponent, ViewportDetailsComponent>()) {
                auto&& [entity, mouse, viewport] = hardwareDetails.value();
                if (ImGui::TreeNodeEx("Window", ImGuiTreeNodeFlags_DefaultOpen)) {
                    viewport.imGuiEditor();
                    ImGui::TreePop();
                }
                if (ImGui::TreeNodeEx("Mouse", ImGuiTreeNodeFlags_DefaultOpen)) {
                    mouse.imGuiEditor();
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        if (ImGui::Button("VSync")) {
            window.toggleVSync();
        }

        // if (ImGui::Checkbox("Show bounding boxes", &mShowBB)) {
        //     if (mShowBB) {
        //         getShader<LineShader>().mActive = true;
        //         for (auto boxEntity : ecs.getView<BoundingBoxComponent>()) {
        //             auto box = ecs.getComponent<BoundingBoxComponent>(boxEntity);
        //             auto line = ecs.getComponent<LineMeshComponent>(boxEntity);
        //             if (!line) {
        //                 line = ecs.addComponent<LineMeshComponent>(boxEntity);

        //                 line->mUseParentSpatial = true;
        //                 line->mWriteDepth = true;
        //                 line->mOverrideColor = util::genRandomVec3(0.3f, 1.f);

        //                 glm::vec3 NearLeftBottom{ box->mMin };
        //                 glm::vec3 NearLeftTop{ box->mMin.x, box->mMax.y, box->mMin.z };
        //                 glm::vec3 NearRightBottom{ box->mMax.x, box->mMin.y, box->mMin.z };
        //                 glm::vec3 NearRightTop{ box->mMax.x, box->mMax.y, box->mMin.z };
        //                 glm::vec3 FarLeftBottom{ box->mMin.x, box->mMin.y,  box->mMax.z };
        //                 glm::vec3 FarLeftTop{ box->mMin.x, box->mMax.y,     box->mMax.z };
        //                 glm::vec3 FarRightBottom{ box->mMax.x, box->mMin.y, box->mMax.z };
        //                 glm::vec3 FarRightTop{ box->mMax };

        //                 line->addNode(NearLeftBottom);
        //                 line->addNode(NearLeftTop);
        //                 line->addNode(NearRightTop);
        //                 line->addNode(NearRightBottom);
        //                 line->addNode(NearLeftBottom);
        //                 line->addNode(FarLeftBottom);
        //                 line->addNode(FarLeftTop);
        //                 line->addNode(NearLeftTop);
        //                 line->addNode(FarLeftTop);
        //                 line->addNode(FarRightTop);
        //                 line->addNode(NearRightTop);
        //                 line->addNode(FarRightTop);
        //                 line->addNode(FarRightBottom);
        //                 line->addNode(NearRightBottom);
        //                 line->addNode(FarRightBottom);
        //                 line->addNode(FarLeftBottom);
        //             }
        //         }
        //     }
        //     else {
        //         for (auto tuple : ecs.getView<BoundingBoxComponent, LineMeshComponent>()) {
        //             ecs.removeComponent<LineMeshComponent>(tuple);
        //         }
        //     }
        // }
        if (ImGui::TreeNodeEx("Shaders", ImGuiTreeNodeFlags_DefaultOpen)) {
//             auto shadersFunc = [&](std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>>& shaders, const std::string swapName) {
//                 for (unsigned i = 0; i < shaders.size(); i++) {
//                     auto& shader = shaders[i];
//                     ImGui::PushID(i);
// 
//                     ImGui::Checkbox("", &shader.second->mActive);
//                     ImGui::SameLine();
//                     bool treeActive = ImGui::TreeNodeEx(shader.second->mName.c_str());
// 
//                     if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
//                         ImGui::SetDragDropPayload(swapName.c_str(), &i, sizeof(unsigned));
//                         ImGui::TextWrapped("Swap %s", shader.second->mName.c_str());
//                         ImGui::EndDragDropSource();
//                     }
//                     if (ImGui::BeginDragDropTarget()) {
//                         if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload(swapName.c_str())) {
//                             IM_ASSERT(payLoad->DataSize == sizeof(unsigned));
//                             unsigned payload_n = *(const unsigned*)payLoad->Data;
//                             shaders[i].swap(shaders[payload_n]);
//                         }
//                         ImGui::EndDragDropTarget();
//                     }
// 
//                     if (treeActive) {
//                         ImGui::Spacing();
//                         ImGui::SameLine();
//                         if (ImGui::Button("Reload")) {
//                             shader.second->reload();
//                         }
//                         shader.second->imguiEditor();
//                         ImGui::TreePop();
//                     }
//                     ImGui::PopID();
//                 }
//             };
// 
            // if (Renderer::mComputeShaders.size() && ImGui::TreeNodeEx("Compute", ImGuiTreeNodeFlags_DefaultOpen)) {
            //     shadersFunc(Renderer::mComputeShaders, "COMPUTE_SWAP");
            //     ImGui::TreePop();
            // }
            // if (Renderer::mPreProcessShaders.size() && ImGui::TreeNodeEx("Pre process", ImGuiTreeNodeFlags_DefaultOpen)) {
            //     shadersFunc(Renderer::mPreProcessShaders, "PRESHADER_SWAP");
            //     ImGui::TreePop();
            // }
            // if (Renderer::mSceneShaders.size() && ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
            //     shadersFunc(Renderer::mSceneShaders, "SCENESHADER_SWAP");
            //     ImGui::TreePop();
            // }
            // if (Renderer::mPostShaders.size() && ImGui::TreeNodeEx("Post process", ImGuiTreeNodeFlags_DefaultOpen)) {
            //     shadersFunc(Renderer::mPostShaders, "POSTSHADER_SWAP");
            //     ImGui::TreePop();
            // }
            ImGui::TreePop();
        }

        ImGui::End();

    }

    void Renderer::clean() {
        // for (auto& shader : mComputeShaders) {
        //     shader.second->cleanUp();
        // }
        // mComputeShaders.clear();
        // for (auto& shader : mPreProcessShaders) {
        //     shader.second->cleanUp();
        // }
        // mPreProcessShaders.clear();
        // for (auto& shader : mSceneShaders) {
        //     shader.second->cleanUp();
        // }
        // mSceneShaders.clear();
        // for (auto& shader : mPostShaders) {
        //     shader.second->cleanUp();
        // }
        // mPostShaders.clear();
        mShowBB = false;

        resetState();
    }


}
