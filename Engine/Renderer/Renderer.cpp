#include "Renderer/pch.hpp"

#include "Renderer.hpp"

#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/RenderingSystems/LineRenderer.hpp"
#include "Renderer/RenderingSystems/Blitter.hpp"
#include "Renderer/RenderingSystems/ImGuiRenderer.hpp"

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
		mDefaultFBOHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"backbuffer",
			FramebufferBuilder{}
			.setSize(glm::uvec2(viewport.mSize))
			.attach(TextureFormat{ types::texture::Target::Texture2D,
				types::texture::InternalFormats::RGB16_UNORM,
				{ types::texture::Filters::Linear, types::texture::Filters::Linear },
				{ types::texture::Wraps::Clamp, types::texture::Wraps::Clamp }
				})
			.attach(TextureFormat{ types::texture::Target::Texture2D,
				types::texture::InternalFormats::D16,
				{ types::texture::Filters::Linear, types::texture::Filters::Linear },
				{ types::texture::Wraps::Clamp, types::texture::Wraps::Clamp }
				}),
			resourceManagers.mTextureManager
		);
		if (!resourceManagers.mFramebufferManager.isValid(mDefaultFBOHandle)) {
			return;
		}

		resetState();
		auto& defaultFbo = resourceManagers.mFramebufferManager.resolve(mDefaultFBOHandle);
		{
			TRACY_GPUN("Draw Demo");
			if (mWireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			demo->render(resourceManagers, ecs, defaultFbo);
			resetState();
		}

		if (mShowBoundingBoxes) {
			TRACY_GPUN("Debug Draws");
			defaultFbo.bind();
			glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
			drawLines<DebugBoundingBoxComponent>(resourceManagers, ecs, std::get<0>(*ecs.cGetComponent<MainCameraComponent>()));
		}

		/* Render imgui */
		if (!ServiceLocator<ImGuiManager>::empty() && ServiceLocator<ImGuiManager>::ref().isEnabled()) {
			TRACY_GPUN("ImGui Render");
			resetState();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, window.getDetails().mSize.x, window.getDetails().mSize.y);
			glClear(GL_COLOR_BUFFER_BIT);
			drawImGui(resourceManagers, ecs, window.getDetails().mPos, window.getDetails().mSize);
		}
		else {
			TRACY_GPUN("Final Blit");
			Framebuffer fb; // empty framebuffer is just the backbuffer -- just don't do anything with it ever
			blit(resourceManagers, fb, defaultFbo.mTextures[0], window.getDetails().mSize, glm::vec4(0.f, 0.f, 0.f, 1.f));
		}
	}

	void Renderer::_imGuiEditor(WindowSurface& window, ECS& ecs, ResourceManagers& resourceManager) {
		TRACY_ZONE();
		NEO_UNUSED(ecs);

		ImGui::Begin("Viewport");
		ServiceLocator<ImGuiManager>::ref().updateViewport();
		glm::vec2 viewportSize = ServiceLocator<ImGuiManager>::ref().getViewportSize();
		if (viewportSize.x != 0 && viewportSize.y != 0) {
			if (resourceManager.mFramebufferManager.isValid(mDefaultFBOHandle)) {
				auto& defaultFbo = resourceManager.mFramebufferManager.resolve(mDefaultFBOHandle);
#pragma warning(push)
#pragma warning(disable: 4312)
				ImGui::Image(defaultFbo.mTextures[0].mHandle, {viewportSize.x, viewportSize.y}, ImVec2(0, 1), ImVec2(1, 0));
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
