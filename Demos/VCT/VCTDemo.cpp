#include "VCT/VCTDemo.hpp"

#include "VoxelTypes.hpp"
#include "VolumeComponent.hpp"
#include "Voxelizer.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/EngineComponents/PinnedComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/WireframeRenderComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"

#include "ECS/Component/RenderingComponent/ForwardPBRRenderComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumCullingSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumToLineSystem.hpp"
#include "ECS/Systems/CameraSystems/CSMFittingSystem.hpp"

#include "Renderer/RenderingSystems/Blitter.hpp"
#include "Renderer/RenderingSystems/ForwardPBRRenderer.hpp"
#include "Renderer/RenderingSystems/PointLightShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/WireframeRenderer.hpp"
#include "Renderer/RenderingSystems/TonemapRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "Loader/MeshGenerator.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */

namespace VCT {
	namespace {

		inline void destroyScene(ECS& ecs, ResourceManagers& resourceManagers) {
			if (auto volumeView = ecs.getSingleView<VolumeComponent, SpatialComponent, BoundingBoxComponent>()) {
				ecs.removeEntity(std::get<ECS::Entity>(*volumeView));
			}

			if (auto lightView = ecs.getSingleView<LightComponent, SpatialComponent>()) {
				ecs.removeEntity(std::get<ECS::Entity>(*lightView));
				if (ecs.getComponent<PointLightShadowMapComponent>(std::get<ECS::Entity>(*lightView))) {
					resourceManagers.mTextureManager.discard(ecs.cGetComponent<PointLightShadowMapComponent>(std::get<ECS::Entity>(*lightView))->mShadowMap);
				}
				if (ecs.getComponent<CSMShadowMapComponent>(std::get<ECS::Entity>(*lightView))) {
					resourceManagers.mTextureManager.discard(ecs.cGetComponent<CSMShadowMapComponent>(std::get<ECS::Entity>(*lightView))->mShadowMap);
				}
			}
			if (auto csmCamera0 = ecs.getSingleView<SpatialComponent, CSMCamera0Component>()) {
				ecs.removeEntity(std::get<ECS::Entity>(*csmCamera0));
			}
			if (auto csmCamera1 = ecs.getSingleView<SpatialComponent, CSMCamera1Component>()) {
				ecs.removeEntity(std::get<ECS::Entity>(*csmCamera1));
			}
			if (auto csmCamera2 = ecs.getSingleView<SpatialComponent, CSMCamera2Component>()) {
				ecs.removeEntity(std::get<ECS::Entity>(*csmCamera2));
			}

			const auto& meshView = ecs.getView<VoxelizeComponent, MeshComponent, SpatialComponent>();
			for (auto entity : meshView) {
				ecs.removeEntity(entity);
			}
		}

		inline void createVolume(ECS& ecs, glm::vec3 pos, glm::vec3 scale) {
			SpatialComponent volumeSpatial(pos, scale);
			volumeSpatial.setLookDir(glm::vec3(0, 0, -1));
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<PinnedComponent>()
				.attachComponent<TagComponent>("Volume")
				.attachComponent<SpatialComponent>(volumeSpatial)
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f), true)
				.attachComponent<WireframeRenderComponent>(glm::vec3(1.f))
				.attachComponent<VolumeComponent>()
				.attachComponent<MeshComponent>(MeshHandle("cube"))
				.attachComponent<CameraComponent>(-0.5f, 0.5f, CameraComponent::Orthographic{ glm::vec2(-0.5f, 0.5f), glm::vec2(-0.5f, 0.5f) }) // Dealt with later
			));
		}

		inline void createCornellScene(ECS& ecs, ResourceManagers& resourceManagers) {
			destroyScene(ecs, resourceManagers);

			auto createEntity = [](std::string name, MeshHandle meshHandle, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, glm::vec3 color) {
				MaterialComponent material;
				material.mAlbedoColor = glm::vec4(color.x, color.y, color.z, 1.f);
				return std::move(ECS::EntityBuilder{}
					.attachComponent<TagComponent>(name)
					.attachComponent<MeshComponent>(meshHandle)
					.attachComponent<MaterialComponent>(material)
					.attachComponent<SpatialComponent>(position, scale, rotation)
					.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
					.attachComponent<ForwardPBRRenderComponent>()
					.attachComponent<OpaqueComponent>()
					.attachComponent<ShadowCasterRenderComponent>()
					.attachComponent<VoxelizeComponent>()
				);
				};

			createVolume(ecs, glm::vec3(0.f, 2.5f, 2.5f), glm::vec3(5.f));

			{
				PointLightShadowMapComponent shadowMap(512, resourceManagers.mTextureManager);
				ecs.submitEntity(std::move(ECS::EntityBuilder{}
					.attachComponent<TagComponent>("Light")
					.attachComponent<SpatialComponent>(glm::vec3(0.f, 5.f - util::EP * 3, 2.5f), glm::vec3(30.f))
					.attachComponent<MainLightComponent>()
					.attachComponent<LightComponent>(glm::vec3(1.f), 150.f)
					.attachComponent<PointLightComponent>()
					.attachComponent<PointLightShadowMapComponent>(shadowMap)
					.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f), true)
				));
			}

			// Cornell box
			{
				HashedString quadMesh("quad");
				ecs.submitEntity(std::move(createEntity("backwall", quadMesh, glm::vec3(0.f, 2.45f, 0.f), glm::vec3(5.f, 5.f, 0.05f), glm::vec3(0.f), glm::vec3(1.f))));
				ecs.submitEntity(std::move(createEntity("leftwall", quadMesh, glm::vec3(-2.45f, 2.5f, 2.5f), glm::vec3(5.f, 5.f, 0.05f), glm::vec3(0.f, glm::radians(90.f), 0.f), glm::vec3(1.f, 0.f, 0.f))));
				ecs.submitEntity(std::move(createEntity("rightwall", quadMesh, glm::vec3(2.45f, 2.5f, 2.5f), glm::vec3(5.f, 5.f, 0.05f), glm::vec3(0.f, glm::radians(-90.f), 0.f), glm::vec3(0.f, 1.f, 0.f))));
				ecs.submitEntity(std::move(createEntity("floor", quadMesh, glm::vec3(0.f, 0.05f, 2.5f), glm::vec3(5.f, 5.f, 0.05f), glm::vec3(glm::radians(-90.f), 0.f, 0.f), glm::vec3(1.f))));
				ecs.submitEntity(std::move(createEntity("ceiling", quadMesh, glm::vec3(0.f, 4.95f, 2.5f), glm::vec3(5.f, 5.f, 0.05f), glm::vec3(glm::radians(90.f), 0.f, 0.f), glm::vec3(1.f))));
				ecs.submitEntity(std::move(createEntity("sphere", HashedString("sphere"), glm::vec3(1.25f, 0.85f, 3.0f), glm::vec3(1.5f), glm::vec3(0.f), glm::vec3(1.f))));
				auto&& box = createEntity("box1", HashedString("cube"), glm::vec3(-0.85f, 1.5f, 2.5f), glm::vec3(1.25f, 3.f, 1.25f), glm::vec3(0.f, glm::radians(33.f), 0.f), glm::vec3(1.f));
				box.attachComponent<RotationComponent>(glm::vec3(0.f, 1.0f, 0.f));
				ecs.submitEntity(std::move(box));
			}
		}

		inline void createSphereScene(ECS& ecs, ResourceManagers& resourceManagers) {
			destroyScene(ecs, resourceManagers);

			createVolume(ecs, glm::vec3(0.f, 2.5f, 2.5f), glm::vec3(5.f));

			SpatialComponent spatial(glm::vec3(10, 10, 0), glm::vec3(1.f));
			spatial.setLookDir(glm::vec3(-0.5f, -0.7f, -0.4f));
			CSMShadowMapComponent csmShadowMap(512, resourceManagers.mTextureManager);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Light")
				.attachComponent<LightComponent>(glm::vec3(1.f))
				.attachComponent<DirectionalLightComponent>()
				.attachComponent<SpatialComponent>(spatial)
				.attachComponent<MainLightComponent>()
				.attachComponent<CameraComponent>(-2.f, 2.f, CameraComponent::Orthographic{ glm::vec2(-4.f, 2.f), glm::vec2(0.1f, 5.f) })
				.attachComponent<FrustumComponent>()
				.attachComponent<FrustumFitReceiverComponent>()
				.attachComponent<CSMShadowMapComponent>(csmShadowMap)
			));

			{
				auto csmCameras = createCSMCameras();
				for (int i = 0; i < csmCameras.size(); i++) {
					ecs.submitEntity(std::move(csmCameras[i]
						.attachComponent<TagComponent>("CSMCamera" + std::to_string(i))
					));
				}
			}

			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(1.0);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Sphere")
				.attachComponent<MeshComponent>(MeshHandle("sphere"))
				.attachComponent<MaterialComponent>(material)
				.attachComponent<SpatialComponent>(glm::vec3(0.f, 2.5f, 2.5f), glm::vec3(4.5f))
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<ForwardPBRRenderComponent>()
				.attachComponent<OpaqueComponent>()
				.attachComponent<ShadowCasterRenderComponent>()
				.attachComponent<VoxelizeComponent>()
			));
		}

		inline void createSponzaScene(ECS& ecs, ResourceManagers& resourceManagers) {
			destroyScene(ecs, resourceManagers);

			createVolume(ecs, glm::vec3(-1.f, 9.f, -0.5f), glm::vec3(35.5f, 21.f, 17.3f));

			SpatialComponent spatial(glm::vec3(75.f, 200.f, 20.f));
			spatial.setLookDir(glm::normalize(glm::vec3(-0.28f, -0.96f, -0.06f)));
			CSMShadowMapComponent csmShadowMap(2048, resourceManagers.mTextureManager);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Light")
				.attachComponent<LightComponent>(glm::vec3(0.978f, 0.903f, 0.714f), 3.f)
				.attachComponent<DirectionalLightComponent>()
				.attachComponent<SpatialComponent>(spatial)
				.attachComponent<MainLightComponent>()
				.attachComponent<CameraComponent>(-2.f, 2.f, CameraComponent::Orthographic{ glm::vec2(-4.f, 2.f), glm::vec2(0.1f, 5.f) })
				.attachComponent<FrustumComponent>()
				.attachComponent<FrustumFitReceiverComponent>()
				.attachComponent<CSMShadowMapComponent>(csmShadowMap)
			));

			{
				auto csmCameras = createCSMCameras();
				for (int i = 0; i < csmCameras.size(); i++) {
					ecs.submitEntity(std::move(csmCameras[i]
						.attachComponent<TagComponent>("CSMCamera" + std::to_string(i))
					));
				}
			}

			Loader::loadGltfScene(ecs, resourceManagers, "Sponza/Sponza.gltf", glm::scale(glm::mat4(1.f), glm::vec3(200.f)),
				[](ECS& ecs, const GLTFImporter::MeshNode& node) {
					ECS::EntityBuilder builder;
					if (!node.mName.empty()) {
						builder.attachComponent<TagComponent>(node.mName);
					}
					builder.attachComponent<SpatialComponent>(node.mSpatial);
					builder.attachComponent<MeshComponent>(node.mMeshHandle);
					builder.attachComponent<BoundingBoxComponent>(node.mMin, node.mMax, true);
					if (node.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::Transparent) {
						builder.attachComponent<TransparentComponent>();
						// builder.attachComponent<ForwardPBRRenderComponent>();
					}
					else if (node.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::AlphaTest) {
						builder.attachComponent<AlphaTestComponent>();
						// builder.attachComponent<DeferredPBRRenderComponent>();
					}
					else {
						builder.attachComponent<OpaqueComponent>();
						// builder.attachComponent<DeferredPBRRenderComponent>();
					}
					builder.attachComponent<ForwardPBRRenderComponent>();
					builder.attachComponent<VoxelizeComponent>();
					builder.attachComponent<MaterialComponent>(node.mMaterial);
					builder.attachComponent<ShadowCasterRenderComponent>();
					ecs.submitEntity(std::move(builder));
				});

			auto createSphere = [&](const MaterialComponent& material, glm::vec3 pos, glm::vec3 scale) {
				ecs.submitEntity(std::move(ECS::EntityBuilder{}
					.attachComponent<TagComponent>("Sphere")
					.attachComponent<MeshComponent>(MeshHandle("sphere"))
					.attachComponent<MaterialComponent>(material)
					.attachComponent<SpatialComponent>(pos, scale)
					.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
					.attachComponent<ForwardPBRRenderComponent>()
					.attachComponent<OpaqueComponent>()
					.attachComponent<ShadowCasterRenderComponent>()
					.attachComponent<VoxelizeComponent>()
				));
			};
			{
				MaterialComponent material;
				material.mAlbedoColor = glm::vec4(1.0);
				material.mEmissiveFactor = glm::vec3(10.0, 0.0, 10.0);
				createSphere(material, glm::vec3(14.3f, 1.8f, -5.7f), glm::vec3(1.0f));
			}
			{
				MaterialComponent material;
				material.mAlbedoColor = glm::vec4(1.0);
				material.mEmissiveFactor = glm::vec3(0.0, 10.0, 10.0);
				createSphere(material, glm::vec3(14.3f, 1.8f, 5.15f), glm::vec3(1.0f));
			}
		}
	}

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "VCT";
		return config;
	}

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(resourceManagers);

		/* Camera */
		{
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Camera")
				.attachComponent<SpatialComponent>(glm::vec3(0.f, 3.f, 12.f), glm::vec3(1.f))
				.attachComponent<CameraComponent>(1.f, 100.f, CameraComponent::Perspective{ 45.f, 1.f })
				.attachComponent<CameraControllerComponent>(0.4f, 7.f)
				.attachComponent<MainCameraComponent>()
				.attachComponent<FrustumComponent>()
				.attachComponent<FrustumFitSourceComponent>()
			));
		}

		createCornellScene(ecs, resourceManagers);

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>();
		ecs.addSystem<RotationSystem>();
		ecs.addSystem<FrustumSystem>(); // Calculate original frusta bounds
		ecs.addSystem<CSMFittingSystem>(); // Break scene frustum into slices and fit CSMCameraN to those slices
		ecs.addSystem<FrustumCullingSystem>();
	}

	void Demo::imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs, resourceManagers);
		mAutoExposureParams.imguiEditor();

		if (ImGui::Button("Create Cornell Scene")) {
			createCornellScene(ecs, resourceManagers);
		}
		if (ImGui::Button("Create Sphere Scene")) {
			createSphereScene(ecs, resourceManagers);
		}
		if (ImGui::Button("Create Sponza Scene")) {
			createSponzaScene(ecs, resourceManagers);
		}

		ImGui::Checkbox("Debug Draw", &mDebugDraw);
	}

	void Demo::update(ECS& ecs, ResourceManagers&) {
		if (auto volumeView = ecs.getSingleView<VolumeComponent, SpatialComponent, CameraComponent, BoundingBoxComponent>()) {
			const auto& spatial = std::get<SpatialComponent&>(*volumeView);
			const auto& aabb = std::get<BoundingBoxComponent&>(*volumeView);
			glm::vec3 volumeWorldMin = glm::vec3(spatial.getModelMatrix() * glm::vec4(aabb.mMin, 1.0));
			glm::vec3 volumeWorldMax = glm::vec3(spatial.getModelMatrix() * glm::vec4(aabb.mMax, 1.0));
			glm::vec3 halfExtents = glm::abs(volumeWorldMax - volumeWorldMin) / 2.f;
			auto& camera = std::get<CameraComponent&>(*volumeView);
			camera.setNear(-halfExtents.z);
			camera.setFar(halfExtents.z);
			camera.setOrthographic(CameraComponent::Orthographic{
				glm::vec2(-halfExtents.x, halfExtents.x),
				glm::vec2(-halfExtents.y, halfExtents.y)
				});
		}
	}

	void Demo::render(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs, const TextureHandle& outputColor, const TextureHandle& outputDepth) {
		auto voxelizeBuffers = voxelize(renderPasses, resourceManagers, ecs);

		if (auto lightView = ecs.getSingleView<MainLightComponent, SpatialComponent>()) {
			auto lightEntity = std::get<ECS::Entity>(*lightView);
			if (ecs.has<PointLightShadowMapComponent>(lightEntity)) {
				PointLightShadowMapParameters params = {
					0.01f
				};
				drawPointLightShadows<OpaqueComponent>(renderPasses, resourceManagers, ecs, lightEntity, true, params);
			}
			else if (ecs.has<CSMShadowMapComponent>(lightEntity)) {
				drawCSMShadows(renderPasses, resourceManagers, ecs, lightEntity, true);
			}
		}

		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
		TextureHandle sceneColor = resourceManagers.mTextureManager.asyncLoad("Scene Color",
			TextureBuilder{}
			.setFormat(TextureFormat{ types::texture::Target::Texture2D,
				types::InternalFormats::RGB16_F,
				})
			.setDimension(glm::u16vec3(viewport.mSize.x, viewport.mSize.y, 0))
		);
		if (resourceManagers.mTextureManager.isValid(sceneColor)) {
			const Texture& sceneColorTex = resourceManagers.mTextureManager.resolve(sceneColor);
			if (sceneColorTex.mWidth != viewport.mSize.x || sceneColorTex.mHeight != viewport.mSize.y) {
				resourceManagers.mTextureManager.discard(sceneColor);
				return;
			}
		}

		auto sceneTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"Scene Target",
			FramebufferExternalAttachments{
				FramebufferAttachment{sceneColor},
				FramebufferAttachment{outputDepth},
			},
			resourceManagers.mTextureManager
		);

		renderPasses.clear(sceneTargetHandle, types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth, glm::vec4(0.f, 0.f, 0.f, 1.f));
		const auto [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();

		if (mDebugDraw) {
			drawWireframe<VolumeComponent>(sceneTargetHandle, viewport.mSize, renderPasses, cameraEntity);
			debugVoxelNodes(sceneTargetHandle, viewport.mSize, renderPasses, ecs, voxelizeBuffers, cameraEntity);
		}
		else {
			drawForwardPBR<OpaqueComponent>(renderPasses, sceneTargetHandle, viewport.mSize, cameraEntity);
			drawForwardPBR<AlphaTestComponent>(renderPasses, sceneTargetHandle, viewport.mSize, cameraEntity);
		}

		auto previousHDRColorHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"Previous HDR Color",
			FramebufferBuilder{}
			.setSize(viewport.mSize)
			.attach(TextureFormat{ types::texture::Target::Texture2D, types::InternalFormats::RGBA16_F }),
			resourceManagers.mTextureManager
		);
		blit(renderPasses, previousHDRColorHandle, viewport.mSize, sceneColor, "Blit Previous HDR Color");

		TextureHandle averageLuminance = NEO_INVALID_HANDLE;
		if (resourceManagers.mFramebufferManager.isValid(previousHDRColorHandle)) {
			averageLuminance = calculateAutoexposure(renderPasses, resourceManagers, ecs, resourceManagers.mFramebufferManager.resolve(previousHDRColorHandle).mTextures[0], mAutoExposureParams);
		}
		TextureHandle tonemappedHandle = tonemap(renderPasses, resourceManagers, viewport.mSize, sceneColor, averageLuminance);

		auto outputTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"FXAA Target",
			FramebufferExternalAttachments{
				FramebufferAttachment{outputColor},
			},
			resourceManagers.mTextureManager
			);
		renderPasses.clear(outputTargetHandle, types::framebuffer::AttachmentBit::Color, glm::vec4(0.f, 0.f, 0.f, 1.f), "Clear Output");
		drawFXAA(renderPasses, outputTargetHandle, viewport.mSize, tonemappedHandle);
	}

	void Demo::destroy() {
	}

}
