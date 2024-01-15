#include "Renderer/pch.hpp"
#include "Renderer.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"

#include "Messaging/Message.hpp"
#include "Messaging/Messenger.hpp"

#include "Engine/Engine.hpp"
#include "Engine/ImGuiManager.hpp"
#include "Hardware/WindowSurface.hpp"

#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"

#include <GLFW/glfw3.h>
#include <imgui_impl_opengl3.h>
#include <tracy/TracyOpenGL.hpp>

namespace neo {

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
        NEO_UNUSED(config);
    }

    void Renderer::init() {
	#ifdef DEBUG_MODE
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GLHelper::OpenGLMessageCallback, nullptr);
		
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		glDebugMessageControl(GL_DEBUG_SOURCE_APPLICATION, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
		glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	#endif

        /* Init default FBO */
        glActiveTexture(GL_TEXTURE0);
        mBackBuffer = Library::createFBO("0");
        mBackBuffer->mFBOID = 0;

        mDefaultFBO = Library::createFBO("backbuffer");
        NewTextureFormat format = { TextureTarget::Texture2D, GL_RGB16, GL_RGB, GL_LINEAR, GL_CLAMP_TO_EDGE };
        mDefaultFBO->attachColorTexture({ 1, 1 }, format);
        mDefaultFBO->attachDepthTexture({ 1, 1 }, GL_LINEAR, GL_CLAMP_TO_EDGE);
        mDefaultFBO->initDrawBuffers();
        mDefaultFBO->bind();

        mBlitShader = new SourceShader("Final Blit", SourceShader::ShaderCode{
            { ShaderStage::VERTEX,
            R"(
                layout (location = 0) in vec3 vertPos;
                layout (location = 2) in vec2 vertTex;
                out vec2 fragTex;
                void main() { 
                    gl_Position = vec4(2 * vertPos, 1); 
                    fragTex = vertTex; 
                } 
            )"},
            { ShaderStage::FRAGMENT,
            R"(
                in vec2 fragTex;
                layout (binding = 0) uniform sampler2D inputTexture;
                out vec4 color;
                void main() {
                    color = texture(inputTexture, fragTex);
                }
            )"}
        });

        
        /* Set max work group */
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &mDetails.mMaxComputeWorkGroupSize.x);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &mDetails.mMaxComputeWorkGroupSize.y);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &mDetails.mMaxComputeWorkGroupSize.z);
        char buf[512];
        memcpy(buf, glGetString(GL_VENDOR), 512);
        mDetails.mVendor = buf;
        memcpy(buf, glGetString(GL_RENDERER), 512);
        mDetails.mRenderer = buf;
        memcpy(buf, glGetString(GL_SHADING_LANGUAGE_VERSION), 512);
        mDetails.mShadingLanguage = buf;

        /* Init default GL state */
        resetState();

        Messenger::removeReceiver<FrameSizeMessage>(this);
        Messenger::addReceiver<FrameSizeMessage, &Renderer::_onFrameSizeChanged>(this);
    }

    void Renderer::_onFrameSizeChanged(const FrameSizeMessage& msg) {
        mDefaultFBO->destroy();
        NewTextureFormat format = { TextureTarget::Texture2D, GL_RGB16, GL_RGB, GL_LINEAR, GL_CLAMP_TO_EDGE };
        mDefaultFBO->attachColorTexture({ msg.mSize.x, msg.mSize.y }, format);
        mDefaultFBO->attachDepthTexture({ msg.mSize.x, msg.mSize.y }, GL_LINEAR, GL_CLAMP_TO_EDGE);
        mDefaultFBO->initDrawBuffers();
        mDefaultFBO->bind();
    }

    void Renderer::resetState() {
        TRACY_GPU();

        glClearColor(0.0f, 0.0f, 0.0f, 1.f);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glDisable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindVertexArray(0);
    }

    void Renderer::render(WindowSurface& window, IDemo* demo, ECS& ecs) {
        TRACY_GPU();

        mStats = {};
        resetState();
        mDefaultFBO->bind();

        {
            TRACY_GPUN("Draw Demo");
            demo->render(ecs, *mDefaultFBO);
        }
        
        /* Render imgui */
        if (!ServiceLocator<ImGuiManager>::empty() && ServiceLocator<ImGuiManager>::ref().isEnabled()) {
            TRACY_GPUN("ImGuiManager.render");
            mBackBuffer->bind();
            ServiceLocator<ImGuiManager>::ref().render();
        }
        else {
            TRACY_GPUN("Final Blit");
            mBackBuffer->bind();
            resetState();
            glDisable(GL_DEPTH_TEST);
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT);
            auto frameSize = window.getDetails().mSize;
            glViewport(0, 0, frameSize.x, frameSize.y);
        
            auto& resolvedBlit = mBlitShader->getResolvedInstance({});
            resolvedBlit.bind();
        
            auto meshData = Library::getMesh("quad");
            glBindVertexArray(meshData.mMesh->mVAOID);
        
            // Bind input fbo texture
            resolvedBlit.bindTexture("inputTexture", *mDefaultFBO->mTextures[0]);
        
            // Render 
            meshData.mMesh->draw();
            resolvedBlit.unbind();
        }
    }

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
            ImGui::TextWrapped("Num Triangles: %d", mStats.mNumTriangles);
            ImGui::TextWrapped("Num Uniforms: %d", mStats.mNumUniforms);
            ImGui::TextWrapped("Num Samplers: %d", mStats.mNumSamplers);
            ImGui::TreePop();
        }
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

        if (ImGui::Button("VSync")) {
            window.toggleVSync();
        }

        ImGui::End();

    }

    void Renderer::clean() {

        resetState();
    }


}
