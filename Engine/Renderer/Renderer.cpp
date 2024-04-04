#include "Renderer/pch.hpp"

#include "Renderer.hpp"

#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/RenderingSystems/LineRenderer.hpp"
#include "Renderer/RenderingSystems/Blitter.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/EngineComponents/DebugBoundingBox.hpp"
#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"

#include "Messaging/Message.hpp"
#include "Messaging/Messenger.hpp"

#include "Engine/Engine.hpp"
#include "Engine/ImGuiManager.hpp"
#include "Hardware/WindowSurface.hpp"

#include <GLFW/glfw3.h>
#include <imgui_impl_opengl3.h>
#include <tracy/TracyOpenGL.hpp>
	
#pragma warning( push )
#pragma warning( disable : 4201 )
#include <glm/gtx/matrix_major_storage.hpp>
#include <glm/gtc/quaternion.hpp>
#pragma warning( pop )

#include <ImGuizmo.h>

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

		mDefaultFBO = Library::createFramebuffer("backbuffer");
		mDefaultFBO->attachColorTexture({ 1, 1 }, { 
			types::texture::Target::Texture2D, 
			types::texture::InternalFormats::RGB16_UNORM,
			{
				types::texture::Filters::Linear,
				types::texture::Filters::Linear
			},
			{
				types::texture::Wraps::Clamp,
				types::texture::Wraps::Clamp
			}
		});
		mDefaultFBO->attachDepthTexture({ 1, 1 }, 
			types::texture::InternalFormats::D16,
			{
				types::texture::Filters::Linear,
				types::texture::Filters::Linear
			},
			{
				types::texture::Wraps::Clamp,
				types::texture::Wraps::Clamp
			}
		);
		mDefaultFBO->initDrawBuffers();
		mDefaultFBO->bind();

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
		TextureFormat colorFormat = mDefaultFBO->mTextures[0]->mFormat;
		TextureFormat depthFormat = mDefaultFBO->mTextures[1]->mFormat;
		mDefaultFBO->destroy();
		mDefaultFBO->init();
		mDefaultFBO->attachColorTexture({ msg.mSize.x, msg.mSize.y }, colorFormat);
		mDefaultFBO->attachDepthTexture({ msg.mSize.x, msg.mSize.y }, depthFormat.mInternalFormat, depthFormat.mFilter, depthFormat.mWrap);
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
		glUseProgram(0);
	}

	void Renderer::render(WindowSurface& window, IDemo* demo, ECS& ecs) {
		TRACY_GPU();

		mStats = {};
		resetState();
		mDefaultFBO->bind();

		{
			TRACY_GPUN("Draw Demo");
			demo->render(ecs, *mDefaultFBO);
			resetState();
		}

		if (mShowBoundingBoxes) {
			TRACY_GPUN("Debug Draws");
			mDefaultFBO->bind();
			drawLines<DebugBoundingBoxComponent>(ecs, std::get<0>(*ecs.getComponent<MainCameraComponent>()));
		}
		
		/* Render imgui */
		if (!ServiceLocator<ImGuiManager>::empty() && ServiceLocator<ImGuiManager>::ref().isEnabled()) {
			{
				TRACY_ZONEN("ImGuizmo::BeginFrame");
				//ImGuizmo::BeginFrame();
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
			}

			TRACY_GPUN("ImGuiManager.render");
			// Bind backbuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			ServiceLocator<ImGuiManager>::ref().render();
		}
		else {
			TRACY_GPUN("Final Blit");
			Framebuffer fb; // empty framebuffer is just the backbuffer -- just don't do anything with it ever
			blit(fb, *mDefaultFBO->mTextures[0], window.getDetails().mSize, glm::vec4(0.f, 0.f, 0.f, 1.f));
		}
	}

	void Renderer::imGuiEditor(WindowSurface& window, ECS& ecs) {
		TRACY_ZONE();
		NEO_UNUSED(ecs);

		ImGui::Begin("Viewport");
		ServiceLocator<ImGuiManager>::ref().updateViewport();
		glm::vec2 viewportSize = ServiceLocator<ImGuiManager>::ref().getViewportSize();
		if (viewportSize.x != 0 && viewportSize.y != 0) {
#pragma warning(push)
#pragma warning(disable: 4312)
			ImGui::Image(reinterpret_cast<ImTextureID>(mDefaultFBO->mTextures[0]->mTextureID), { viewportSize.x, viewportSize.y }, ImVec2(0, 1), ImVec2(1, 0));
#pragma warning(pop)
		}
		ImGuizmo::SetDrawlist();
		const auto&& [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();
		glm::mat4 V = cameraSpatial.getView();
		glm::mat4 P = ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj();
		ecs.getView<BoundingBoxComponent, SpatialComponent>().each([&](BoundingBoxComponent& /* TODO - replace w/ selected comp */, SpatialComponent& spatial) {
			glm::mat4 transform = spatial.getModelMatrix();
			ImGuizmo::Manipulate(
				&V[0][0],
				&P[0][0],
				ImGuizmo::OPERATION::UNIVERSAL,
				ImGuizmo::LOCAL,
				&transform[0][0],
				nullptr,
				nullptr);

			if (ImGuizmo::IsUsing()) {
				glm::vec3 translate, scale, rotate;
				ImGuizmo::DecomposeMatrixToComponents(&transform[0][0], &translate[0], &rotate[0], &scale[0]);
				spatial.setPosition(translate);
				spatial.setScale(scale);
				spatial.setOrientation(glm::mat3(glm::quat(glm::radians(rotate))));
			}
		});

		ImGui::End();

		ImGui::Begin("Renderer");
		if (ImGui::TreeNodeEx("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::TextWrapped("Num Draws: %d", mStats.mNumDraws);
			ImGui::TextWrapped("Num Triangles: %d", mStats.mNumPrimitives);
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

		if (ImGui::Button("Show BoundingBoxes")) {
			mShowBoundingBoxes = !mShowBoundingBoxes;
			if (mShowBoundingBoxes) {
				for (auto boxEntity : ecs.getView<BoundingBoxComponent>()) {
					auto box = ecs.getComponent<BoundingBoxComponent>(boxEntity);
					auto line = ecs.getComponent<LineMeshComponent>(boxEntity);
					if (!line) {
						line = ecs.addComponent<LineMeshComponent>(boxEntity);

						line->mUseParentSpatial = true;
						line->mWriteDepth = true;
						line->mOverrideColor = util::genRandomVec3(0.3f, 1.f);

						glm::vec3 NearLeftBottom{ box->mMin };
						glm::vec3 NearLeftTop{ box->mMin.x, box->mMax.y, box->mMin.z };
						glm::vec3 NearRightBottom{ box->mMax.x, box->mMin.y, box->mMin.z };
						glm::vec3 NearRightTop{ box->mMax.x, box->mMax.y, box->mMin.z };
						glm::vec3 FarLeftBottom{ box->mMin.x, box->mMin.y,  box->mMax.z };
						glm::vec3 FarLeftTop{ box->mMin.x, box->mMax.y,	 box->mMax.z };
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
					ecs.addComponent<DebugBoundingBoxComponent>(boxEntity);
				}
			}
			else {
				for (auto entity : ecs.getView<BoundingBoxComponent, DebugBoundingBoxComponent, LineMeshComponent>()) {
					ecs.removeComponent<DebugBoundingBoxComponent>(entity);
				}
			}
		}

		ImGui::End();

	}

	void Renderer::clean() {

		resetState();
	}


}
