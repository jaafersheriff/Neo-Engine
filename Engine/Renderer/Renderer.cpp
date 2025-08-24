#include "Renderer/pch.hpp"

#include "Renderer.hpp"

#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/RenderingSystems/LineRenderer.hpp"
#include "Renderer/RenderingSystems/Blitter.hpp"
#include "Renderer/RenderingSystems/ImGuiRenderer.hpp"
#include "Renderer/RenderingSystems/RenderPass.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/CollisionComponent/SelectedComponent.hpp"
#include "ECS/Component/EngineComponents/DebugBoundingBox.hpp"
#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"

#include "Messaging/Message.hpp"
#include "Messaging/Messenger.hpp"

#include "Engine/Engine.hpp"
#include "Engine/ImGuiManager.hpp"
#include "Hardware/WindowSurface.hpp"

#include <GLFW/glfw3.h>
#include <tracy/TracyOpenGL.hpp>
	
#pragma warning( push )
#pragma warning( disable : 4201 )
#define GLM_ENABLE_EXPERIMENTAL
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
		mGPUQuery.destroy();
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

		mShowBoundingBoxes = false;

		mGPUQuery.destroy();
		mGPUQuery.init();
	}

	void Renderer::resetState() {
		TRACY_GPU();

		glClearColor(0.0f, 0.0f, 0.0f, 1.f);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glDisable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		glBindVertexArray(0);
		glUseProgram(0);
	}

	void Renderer::render(WindowSurface& window, IDemo* demo, util::Profiler& profiler, const ECS& ecs, ResourceManagers& resourceManagers) {
		TRACY_GPU();
		if (window.isMinimized()) {
			return;
		}

		profiler.markFrameGPU(mGPUQuery.getGPUTime());
		util::Profiler::GPUQuery::Scope _scope(mGPUQuery.tickHandle());

		mStats = {};

		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
		mSceneColorTextureHandle = resourceManagers.mTextureManager.asyncLoad("Main Color",
			TextureBuilder{}
			.setFormat(TextureFormat{ types::texture::Target::Texture2D,
				types::texture::InternalFormats::RGB16_UNORM,
				{ types::texture::Filters::Linear, types::texture::Filters::Linear },
				{ types::texture::Wraps::Clamp, types::texture::Wraps::Clamp }
				})
			.setDimension(glm::u16vec3(viewport.mSize.x, viewport.mSize.y, 0))
		);
		TextureHandle sceneDepthTextureHandle = resourceManagers.mTextureManager.asyncLoad("Main Depth",
			TextureBuilder{}
			.setFormat(TextureFormat{ types::texture::Target::Texture2D,
				types::texture::InternalFormats::D16,
				{ types::texture::Filters::Linear, types::texture::Filters::Linear },
				{ types::texture::Wraps::Clamp, types::texture::Wraps::Clamp }
				})
			.setDimension(glm::u16vec3(viewport.mSize.x, viewport.mSize.y, 0))
		);

		if (!resourceManagers.mTextureManager.isValid(mSceneColorTextureHandle) || resourceManagers.mTextureManager.isValid(sceneDepthTextureHandle)) {
			return;
		}

		{
			TRACY_GPUN("Prepare Frame");
			resetState();
			if (mWireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
		}

		RenderPasses renderPasses;
		{
			TRACY_GPUN("Prepare Demo Draws");
			demo->render(renderPasses, resourceManagers, ecs, mSceneColorTextureHandle, sceneDepthTextureHandle);
		}

		if (mShowBoundingBoxes) {
			auto debugDrawTarget = resourceManagers.mFramebufferManager.asyncLoad(
				"DebugDraw Target",
				FramebufferExternalAttachments{
					FramebufferAttachment{mSceneColorTextureHandle},
					FramebufferAttachment{sceneDepthTextureHandle},
				},
				resourceManagers.mTextureManager
			);
			renderPasses.declarePass(debugDrawTarget, viewport.mSize, [this](const ResourceManagers& resourceManagers, const ECS& ecs) {
				TRACY_GPUN("Debug Draws");
				resetState();
				drawLines<DebugBoundingBoxComponent>(resourceManagers, ecs, std::get<0>(*ecs.cGetComponent<MainCameraComponent>()));
			});
		}

		/* Render imgui */
		if (!ServiceLocator<ImGuiManager>::empty() && ServiceLocator<ImGuiManager>::ref().isEnabled()) {
			// This is kinda garbage and totally ignored
			renderPasses.clear(FramebufferHandle(0), types::framebuffer::AttachmentBit::Color);

			renderPasses.declarePass(FramebufferHandle(0), window.getDetails().mSize, [this, &window](const ResourceManagers& resourceManagers, const ECS& ecs) {
				TRACY_GPUN("ImGui Render");
				resetState();
				drawImGui(resourceManagers, ecs, window.getDetails().mPos, window.getDetails().mSize);
			});
		}
		else {
			renderPasses.clear(FramebufferHandle(0), types::framebuffer::AttachmentBit::Color, glm::vec4(0.f, 0.f, 0.f, 1.f));
			renderPasses.declarePass(FramebufferHandle(0), window.getDetails().mSize, [this](const ResourceManagers& resourceManagers, const ECS&) {
				TRACY_GPUN("Final Blit");
				blit(resourceManagers, mSceneColorTextureHandle);
			});
		}

		renderPasses._execute(resourceManagers, ecs);
	}

	void Renderer::_imGuiEditor(WindowSurface& window, ECS& ecs, ResourceManagers& resourceManager) {
		TRACY_ZONE();
		NEO_UNUSED(ecs);

		ImGui::Begin("Viewport");
		ServiceLocator<ImGuiManager>::ref().updateViewport();
		glm::vec2 viewportSize = ServiceLocator<ImGuiManager>::ref().getViewportSize();
		if (viewportSize.x != 0 && viewportSize.y != 0) {
			if (resourceManager.mTextureManager.isValid(mSceneColorTextureHandle)) {
#pragma warning(push)
#pragma warning(disable: 4312)
				ImGui::Image(mSceneColorTextureHandle.mHandle, {viewportSize.x, viewportSize.y}, ImVec2(0, 1), ImVec2(1, 0));
#pragma warning(pop)
			}
		}
		ImGuizmo::SetDrawlist();
		const auto cameraTuple = ecs.getSingleView<MainCameraComponent, CameraComponent, SpatialComponent>();
		if (cameraTuple) {
			const auto& [cameraEntity, _, camera, cameraSpatial] = *cameraTuple;
			glm::mat4 V = cameraSpatial.getView();
			glm::mat4 P = camera.getProj();
			auto selected = ecs.getSingleView<SelectedComponent, SpatialComponent>();
			if (selected.has_value()) {
				auto&& [selectedEntity, selectedComponent, spatial] = *selected;
				glm::mat4 transform = spatial.getModelMatrix();
				ImGuizmo::Manipulate(
					&V[0][0],
					&P[0][0],
					ImGuizmo::OPERATION::TRANSLATE
					| ImGuizmo::OPERATION::SCALEU
					| ImGuizmo::OPERATION::ROTATE_X | ImGuizmo::OPERATION::ROTATE_Y | ImGuizmo::OPERATION::ROTATE_Z,
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
			}
		}
		ImGui::End();

		///////////////////////////////

		ImGui::Begin("Renderer");
		if (ImGui::TreeNodeEx("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::TextWrapped("Num Draws: %d", mStats.mNumDraws);
			ImGui::TextWrapped("Num Triangles: %d", mStats.mNumPrimitives);
			ImGui::TextWrapped("Num Uniforms: %d", mStats.mNumUniforms);
			ImGui::TextWrapped("Num Samplers: %d", mStats.mNumSamplers);
			ImGui::TreePop();
		}

		if (ImGui::Button("VSync")) {
			window.toggleVSync();
		}

		if (ImGui::Checkbox("Show BoundingBoxes", &mShowBoundingBoxes)) {
			if (mShowBoundingBoxes) {
				for (auto boxEntity : ecs.getView<BoundingBoxComponent>()) {
					ecs.addComponent<DebugBoundingBoxComponent>(boxEntity);
				}
			}
			else {
				for (auto entity : ecs.getView<BoundingBoxComponent, DebugBoundingBoxComponent, LineMeshComponent>()) {
					ecs.removeComponent<DebugBoundingBoxComponent>(entity);
				}
			}
		}
		ImGui::Checkbox("Wireframe", &mWireframe);

		ImGui::End();
	}

	void Renderer::clean() {
		resetState();
	}


}
