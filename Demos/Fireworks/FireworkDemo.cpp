#include "FireworkDemo.hpp"
#include "FireworkComponent.hpp"
#include "FireworkRenderer.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/RenderingComponent/ForwardPBRRenderComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "Renderer/RenderingSystems/Blitter.hpp"
#include "Renderer/RenderingSystems/ForwardPBRRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"
#include "Renderer/RenderingSystems/PointLightShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/TonemapRenderer.hpp"

#include "Loader/GLTFImporter.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"

using namespace neo;

namespace Fireworks {

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "Fireworks Demo";
		return config;
	}

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs, resourceManagers);
		{
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Camera")
				.attachComponent<SpatialComponent>(glm::vec3(0, 0.6f, 5), glm::vec3(1.f))
				.attachComponent<CameraComponent>(1.f, 100.f, CameraComponent::Perspective{ 45.f, 1.f })
				.attachComponent<CameraControllerComponent>(0.4f, 7.f)
				.attachComponent<MainCameraComponent>()
			));
		}

		{
			PointLightShadowMapComponent shadowMap(512, resourceManagers.mTextureManager);
			FireworkComponent firework(resourceManagers.mMeshManager, 16384);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Light")
				.attachComponent<SpatialComponent>(glm::vec3(0.f, 4.f, 7.f), glm::vec3(100.f))
				.attachComponent<LightComponent>(glm::vec3(1.f, 0.25f, 0.f), 1000.f)
				.attachComponent<MainLightComponent>()
				.attachComponent<PointLightComponent>()
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<MeshComponent>(HashedString("sphere"))
				.attachComponent<PointLightShadowMapComponent>(shadowMap)
				.attachComponent<FireworkComponent>(firework)
			));
		}

		{
			Loader::loadGltfScene(ecs, resourceManagers, "bunny.gltf", glm::mat4(1.f), [](ECS& ecs, const GLTFImporter::MeshNode& node) {
				MaterialComponent material = node.mMaterial;
				material.mAlbedoColor = glm::vec4(1.f, 0.f, 1.f, 1.f);
				ecs.submitEntity(std::move(ECS::EntityBuilder{}
					.attachComponent<TagComponent>(node.mName.c_str())
					.attachComponent<SpatialComponent>(glm::vec3(2.f, 0.0f, -1.f), glm::vec3(1.5f))
					.attachComponent<RotationComponent>(glm::vec3(0.f, 1.0f, 0.f))
					.attachComponent<MeshComponent>(node.mMeshHandle)
					.attachComponent<MaterialComponent>(material)
					.attachComponent<BoundingBoxComponent>(node.mMin, node.mMax)
					.attachComponent<ForwardPBRRenderComponent>()
					.attachComponent<OpaqueComponent>()
					.attachComponent<ShadowCasterRenderComponent>()
				));
			});
		}
		{
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(1.f, 1.f, 0.f, 1.f);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Icosahedron")
				.attachComponent<SpatialComponent>(glm::vec3(-2.f, 1.0f, -1.f), glm::vec3(1.5f))
				.attachComponent<RotationComponent>(glm::vec3(1.f, 0.0f, 0.f))
				.attachComponent<MeshComponent>(HashedString("icosahedron"))
				.attachComponent<MaterialComponent>(material)
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<ForwardPBRRenderComponent>()
				.attachComponent<OpaqueComponent>()
				.attachComponent<ShadowCasterRenderComponent>()
			));
		}
		{
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(1.f);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Grid")
				.attachComponent<SpatialComponent>(glm::vec3(0.f), glm::vec3(15.f, 15.f, 1.f), glm::vec3(-util::PI / 2.f, 0.f, 0.f))
				.attachComponent<MeshComponent>(HashedString("quad"))
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f, -0.5f, -0.01f), glm::vec3(0.5f, 0.5f, 0.01f), true)
				.attachComponent<MaterialComponent>(material)
				.attachComponent<ForwardPBRRenderComponent>()
				.attachComponent<OpaqueComponent>()
				.attachComponent<ShadowCasterRenderComponent>()
			));
		}

		ecs.addSystem<CameraControllerSystem>();
		ecs.addSystem<RotationSystem>();
	}

	void Demo::imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs, resourceManagers);
		mBloomParams.imguiEditor();
		mAutoExposureParams.imguiEditor();
	}

	void Demo::update(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs, resourceManagers);
	}

	void Demo::render(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs, const TextureHandle& outputColor, const TextureHandle& outputDepth) {
		renderPasses.computePass([](const ResourceManagers& resourceManagers, const ECS& ecs) {
			tickParticles(resourceManagers, ecs);
		});

		if (const auto& lightView = ecs.getSingleView<MainLightComponent, PointLightComponent, PointLightShadowMapComponent>()) {
			const auto& [lightEntity, _, __, shadowCamera] = *lightView;
			drawPointLightShadows<OpaqueComponent>(renderPasses, resourceManagers, ecs, lightEntity, true);
		}

		 const auto [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();
		 auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());

		TextureHandle hdrColorTexture = resourceManagers.mTextureManager.asyncLoad("HDR Color",
			TextureBuilder{}
			.setDimension(glm::u16vec3(viewport.mSize, 0))
			.setFormat(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGBA16_F })
		);

		auto sceneTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"Scene Target",
		    FramebufferExternalAttachments{
				FramebufferAttachment{hdrColorTexture},
				FramebufferAttachment{outputDepth},
		    }, resourceManagers.mTextureManager
		);
		renderPasses.clear(sceneTargetHandle, types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth, glm::vec4(0.f, 0.f, 0.f, 1.f));
		drawForwardPBR<OpaqueComponent>(renderPasses, sceneTargetHandle, viewport.mSize, cameraEntity);
		renderPasses.renderPass(sceneTargetHandle, viewport.mSize, [cameraEntity](const ResourceManagers& resourceManagers, const ECS& ecs) {
			_drawParticles(resourceManagers, ecs);
		});

 		TextureHandle bloomResults = bloom(renderPasses, resourceManagers, viewport.mSize, hdrColorTexture, mBloomParams);

		auto previousHDRColorHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"Previous HDR Color",
			FramebufferBuilder{}
			.setSize(viewport.mSize)
			.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGBA16_F }),
			resourceManagers.mTextureManager
		);
		{
			RenderState blitState;
			blitState.mDepthState = std::nullopt;
			renderPasses.renderPass(previousHDRColorHandle, viewport.mSize, blitState, [bloomResults](const ResourceManagers& resourceManagers, const ECS&) {
				TRACY_GPUN("Blit Previous HDR Color");
				blit(resourceManagers, bloomResults);
			});
		}

		TextureHandle averageLuminance = NEO_INVALID_HANDLE;
		if (resourceManagers.mFramebufferManager.isValid(previousHDRColorHandle)) {
			averageLuminance = calculateAutoexposure(renderPasses, resourceManagers, ecs, resourceManagers.mFramebufferManager.resolve(previousHDRColorHandle).mTextures[0], mAutoExposureParams);
		}
		TextureHandle tonemappedHandle = tonemap(renderPasses, resourceManagers, viewport.mSize, bloomResults, averageLuminance);

		{
			auto outputTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
				"FXAA Target",
				FramebufferExternalAttachments{
					FramebufferAttachment{outputColor},
				},
				resourceManagers.mTextureManager
				);
			renderPasses.clear(outputTargetHandle, types::framebuffer::AttachmentBit::Color, glm::vec4(0.f, 0.f, 0.f, 1.f), "Clear Output");
			RenderState blitState;
			blitState.mDepthState = std::nullopt;
			renderPasses.renderPass(outputTargetHandle, viewport.mSize, blitState, [tonemappedHandle](const ResourceManagers& resourceManagers, const ECS&) {
				drawFXAA(resourceManagers, tonemappedHandle);
				}, "FXAA");
		}

	}

	void Demo::destroy() {
	}
}
