#include "Sponza/Sponza.hpp"

#include "GBufferComponent.hpp"
#include "GBufferRenderer.hpp"
#include "LightPassRenderer.hpp"
#include "AORenderer.hpp"

#include "Engine/Engine.hpp"
#include "Loader/Loader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/ShadowCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/SpatialComponent/SinTranslateComponent.hpp"
#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"
#include "ECS/Component/RenderingComponent/ShadowCasterRenderComponent.hpp"
#include "ECS/Component/RenderingComponent/WireframeRenderComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumCullingSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustaFittingSystem.hpp"
#include "ECS/Systems/TranslationSystems/SinTranslateSystem.hpp"

#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/RenderingSystems/ShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "Loader/GLTFImporter.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
namespace Sponza {
	namespace {

		struct Camera {
			ECS::Entity mEntity;
			Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
				mEntity = ecs.createEntity();
				ecs.addComponent<TagComponent>(mEntity, "Camera");
				ecs.addComponent<SpatialComponent>(mEntity, pos, glm::vec3(1.f));
				ecs.addComponent<PerspectiveCameraComponent>(mEntity, near, far, fov);
				ecs.addComponent<CameraControllerComponent>(mEntity, ls, ms);
			}
		};

		void _createPointLights(ECS& ecs, const int count) {
			ecs.getView<PointLightComponent>().each([&ecs](auto entity, auto comp) {
				NEO_UNUSED(comp);
				ecs.removeEntity(entity);
			});

			for (int i = 0; i < count; i++) {
				glm::vec3 position(
					util::genRandom(-15.f, 15.f),
					util::genRandom(0.f, 10.f),
					util::genRandom(-7.5f, 7.5f)
				);
				glm::vec3 scale(util::genRandom(1.f, 5.f));
				const auto entity = ecs.createEntity();
				ecs.addComponent<LightComponent>(entity, util::genRandomVec3(0.3f, 1.f));
				ecs.addComponent<PointLightComponent>(entity);
				ecs.addComponent<SinTranslateComponent>(entity, glm::vec3(0.f, util::genRandom(0.f, 5.f), 0.f), position);
				ecs.addComponent<SpatialComponent>(entity, position, scale);
			}
		}
	}

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "Sponza";
		return config;
	}

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {

		/* Game objects */
		Camera camera(ecs, 45.f, 0.1f, 35.f, glm::vec3(0, 0.6f, 5), 0.4f, 15.f);
		ecs.addComponent<MainCameraComponent>(camera.mEntity);
		ecs.addComponent<FrustumComponent>(camera.mEntity);
		ecs.addComponent<FrustumFitSourceComponent>(camera.mEntity);

		{
			auto lightEntity = ecs.createEntity();
			ecs.addComponent<TagComponent>(lightEntity, "Light");
			auto spat = ecs.addComponent<SpatialComponent>(lightEntity, glm::vec3(75.f, 200.f, 20.f));
			spat->setLookDir(glm::normalize(glm::vec3(-0.28f, -0.96f, -0.06f)));
			ecs.addComponent<LightComponent>(lightEntity, glm::vec3(1.f));
			ecs.addComponent<MainLightComponent>(lightEntity);
			ecs.addComponent<DirectionalLightComponent>(lightEntity);
		}
		{
			auto shadowCam = ecs.createEntity();
			ecs.addComponent<TagComponent>(shadowCam, "Shadow Camera");
			ecs.addComponent<OrthoCameraComponent>(shadowCam, -1.f, 1000.f, -100.f, 100.f, -100.f, 100.f);
			ecs.addComponent<ShadowCameraComponent>(shadowCam);
			ecs.addComponent<FrustumComponent>(shadowCam);
			ecs.addComponent<SpatialComponent>(shadowCam);
			ecs.addComponent<FrustumFitReceiverComponent>(shadowCam, 1.f);
		}

		GLTFImporter::Scene scene = Loader::loadGltfScene(resourceManagers, "Sponza/Sponza.gltf", glm::scale(glm::mat4(1.f), glm::vec3(200.f)));
		for (auto& node : scene.mMeshNodes) {
			auto entity = ecs.createEntity();
			if (!node.mName.empty()) {
				ecs.addComponent<TagComponent>(entity, node.mName);
			}
			ecs.addComponent<SpatialComponent>(entity, node.mSpatial);
			ecs.addComponent<MeshComponent>(entity, node.mMeshHandle);
			ecs.addComponent<BoundingBoxComponent>(entity, node.mMin, node.mMax, true);
			if (node.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::Opaque) {
				ecs.addComponent<OpaqueComponent>(entity);
			}
			else if (node.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::AlphaTest) {
				ecs.addComponent<AlphaTestComponent>(entity);
			}
			ecs.addComponent<MaterialComponent>(entity, node.mMaterial);

			ecs.addComponent<ShadowCasterRenderComponent>(entity);
			ecs.addComponent<GBufferRenderComponent>(entity);
			ecs.addComponent<PhongRenderComponent>(entity);
		}

		/* Systems - order matters! */
		auto& camSys = ecs.addSystem<CameraControllerSystem>();
		camSys.mSuperSpeed = 3.f;
		ecs.addSystem<SinTranslateSystem>();
		ecs.addSystem<FrustumSystem>();
		ecs.addSystem<FrustaFittingSystem>();
		ecs.addSystem<FrustumCullingSystem>();
	}

	void Demo::imGuiEditor(ECS& ecs) {
		ImGui::Checkbox("Shadows", &mDrawShadows);

		if (ImGui::Checkbox("Deferred Shading", &mDeferredShading)) {
			if (mDeferredShading) {
				_createPointLights(ecs, mPointLightCount);
			}
		}
		if (mDeferredShading) {
			ImGui::SliderFloat("Debug Radius", &mLightDebugRadius, 0.f, 10.f);
			if (ImGui::SliderInt("# Point Lights", &mPointLightCount, 0, 100)) {
				_createPointLights(ecs, mPointLightCount);
			}

			ImGui::Checkbox("AO", &mDrawAO);
			if (mDrawAO) {
				ImGui::SliderFloat("AO Radius", &mAORadius, 0.f, 1.f);
				ImGui::SliderFloat("AO Bias", &mAOBias, 0.f, 1.f);
			}
		}
	}

	void Demo::render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) {
		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
		FramebufferHandle sceneTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"Scene Target",
			FramebufferBuilder{}
				.setSize(viewport.mSize)
				.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGB16_UNORM })
				.attach(TextureFormat{ types::texture::Target::Texture2D,types::texture::InternalFormats::D16 }),
			resourceManagers.mTextureManager
		);

		TextureHandle shadowTextureHandle;
		if (mDrawShadows) {
			shadowTextureHandle = resourceManagers.mTextureManager.asyncLoad("Shadow map",
				TextureBuilder{}
					.setDimension(glm::u16vec3(2048, 2048, 0))
					.setFormat(TextureFormat{types::texture::Target::Texture2D, types::texture::InternalFormats::D16})
			);
			FramebufferHandle shadowTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
				"Shadow map",
				FramebufferExternal{ {shadowTextureHandle} },
				resourceManagers.mTextureManager
			);

			if (resourceManagers.mFramebufferManager.isValid(shadowTargetHandle)) {
				auto& shadowMap = resourceManagers.mFramebufferManager.resolve(shadowTargetHandle);
				shadowMap.clear(glm::uvec4(0.f, 0.f, 0.f, 0.f), types::framebuffer::AttachmentBit::Depth);
				drawShadows<OpaqueComponent>(resourceManagers, shadowMap, ecs);
				drawShadows<AlphaTestComponent>(resourceManagers, shadowMap, ecs);
			}
		}

		if (resourceManagers.mFramebufferManager.isValid(sceneTargetHandle)) {
			auto& sceneTarget = resourceManagers.mFramebufferManager.resolve(sceneTargetHandle);
			if (mDeferredShading) {
				_deferredShading(resourceManagers, ecs, sceneTargetHandle, viewport.mSize, shadowTextureHandle);
			}
			else {
				_forwardShading(resourceManagers, ecs, sceneTargetHandle, viewport.mSize, shadowTextureHandle);
			}

			backbuffer.bind();
			backbuffer.clear(glm::vec4(0, 0, 0, 1.f), types::framebuffer::AttachmentBit::Color);
			drawFXAA(resourceManagers, glm::uvec2(viewport.mSize.x, viewport.mSize.y), sceneTarget.mTextures[0]);
			// Don't forget the depth. Because reasons.
			glBlitNamedFramebuffer(sceneTarget.mFBOID, backbuffer.mFBOID,
				0, 0, viewport.mSize.x, viewport.mSize.y,
				0, 0, viewport.mSize.x, viewport.mSize.y,
				GL_DEPTH_BUFFER_BIT,
				GL_NEAREST
			);
		}
	}

	void Demo::_forwardShading(
		const ResourceManagers& resourceManagers, 
		const ECS& ecs, 
		FramebufferHandle sceneTargetHandle, 
		glm::uvec2 viewport,
		TextureHandle shadowMapHandle
	) {
		TRACY_GPU();
		const auto&& [cameraEntity, _, __] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();

		auto& sceneTarget = resourceManagers.mFramebufferManager.resolve(sceneTargetHandle);
		sceneTarget.clear(glm::vec4(getConfig().clearColor, 0.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
		glViewport(0, 0, viewport.x, viewport.y);

		drawPhong<OpaqueComponent>(resourceManagers, ecs, cameraEntity, shadowMapHandle);
		drawPhong<AlphaTestComponent>(resourceManagers, ecs, cameraEntity, shadowMapHandle);
	}

	void Demo::_deferredShading(
		const ResourceManagers& resourceManagers, 
		const ECS& ecs, 
		FramebufferHandle sceneTargetHandle, 
		glm::uvec2 viewport, 
		TextureHandle shadowMapHandle
	) {
		TRACY_GPU();
		const auto&& [cameraEntity, _, __] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();
		FramebufferHandle gbufferHandle = createGBuffer(resourceManagers, viewport);
		if (!resourceManagers.mFramebufferManager.isValid(gbufferHandle)) {
			return;
		}
		auto& gbuffer = resourceManagers.mFramebufferManager.resolve(gbufferHandle);
		gbuffer.bind();
		gbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
		glViewport(0, 0, viewport.x, viewport.y);
		drawGBuffer<OpaqueComponent>(resourceManagers, ecs, cameraEntity, {});
		drawGBuffer<AlphaTestComponent>(resourceManagers, ecs, cameraEntity, {});

		TextureHandle aoHandle = NEO_INVALID_HANDLE;
		if (mDrawAO) {
			aoHandle = drawAO(resourceManagers, ecs, cameraEntity, gbuffer, viewport, mAORadius, mAOBias);
		}

		auto lightResolveHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"Light Resolve",
			FramebufferBuilder{}
				.setSize(viewport)
				.attach(TextureFormat{ types::texture::Target::Texture2D,types::texture::InternalFormats::RGB16_UNORM }),
			resourceManagers.mTextureManager
		);

		if (!resourceManagers.mFramebufferManager.isValid(lightResolveHandle)) {
			return;
		}

		auto& lightResolve = resourceManagers.mFramebufferManager.resolve(lightResolveHandle);
		lightResolve.bind();
		lightResolve.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color);
		glViewport(0, 0, viewport.x, viewport.y);
		drawPointLights(resourceManagers, ecs, gbuffer, cameraEntity, viewport, mLightDebugRadius);
		drawDirectionalLights(resourceManagers, ecs, cameraEntity, gbuffer, shadowMapHandle);

		// I'm lazy so I'm just going to do the final combine here
		{
			TRACY_GPUN("Final Combine");
			auto& sceneTarget = resourceManagers.mFramebufferManager.resolve(sceneTargetHandle);
			sceneTarget.bind();
			sceneTarget.clear(glm::vec4(0.f, 0.f, 0.f, 0.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
			glViewport(0, 0, viewport.x, viewport.y);
			auto combineShaderHandle = resourceManagers.mShaderManager.asyncLoad("FinalCombine", SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "quad.vert"},
				{ types::shader::Stage::Fragment, "sponza/combine.frag" }
				});
			if (!resourceManagers.mShaderManager.isValid(combineShaderHandle)) {
				return;
			}
			ShaderDefines defines;
			MakeDefine(DRAW_AO);
			if (mDrawAO && resourceManagers.mTextureManager.isValid(aoHandle)) {
				defines.set(DRAW_AO);
			}
			auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(combineShaderHandle, defines);
			resolvedShader.bind();

			resolvedShader.bindTexture("lightOutput", resourceManagers.mTextureManager.resolve(lightResolve.mTextures[0]));
			if (mDrawAO && resourceManagers.mTextureManager.isValid(aoHandle)) {
				resolvedShader.bindTexture("aoOutput", resourceManagers.mTextureManager.resolve(aoHandle));
			}

			resourceManagers.mMeshManager.resolve("quad").draw();

			// Don't forget the depth. Because reasons.
			glBlitNamedFramebuffer(gbuffer.mFBOID, sceneTarget.mFBOID,
				0, 0, viewport.x, viewport.y,
				0, 0, viewport.x, viewport.y,
				GL_DEPTH_BUFFER_BIT,
				GL_NEAREST
			);
		}
	}
}