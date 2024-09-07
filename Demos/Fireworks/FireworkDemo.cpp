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
		config.name = "Fireworks";
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
			ShadowCameraComponent shadowCamera(types::texture::Target::TextureCube, 512, resourceManagers.mTextureManager);
			FireworkComponent firework(resourceManagers.mMeshManager, 16384);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Light")
				.attachComponent<SpatialComponent>(glm::vec3(0.f, 4.f, 7.f), glm::vec3(100.f))
				.attachComponent<LightComponent>(glm::vec3(1.f, 0.25f, 0.f), 1000.f)
				.attachComponent<MainLightComponent>()
				.attachComponent<PointLightComponent>()
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<MeshComponent>(HashedString("sphere"))
				.attachComponent<ShadowCameraComponent>(shadowCamera)
				.attachComponent<FireworkComponent>(firework)
			));
		}

		{
			GLTFImporter::Scene gltfScene = Loader::loadGltfScene(resourceManagers, "bunny.gltf");
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(1.f, 0.f, 1.f, 1.f);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Bunny")
				.attachComponent<SpatialComponent>(glm::vec3(2.f, 0.0f, -1.f), glm::vec3(1.5f))
				.attachComponent<RotationComponent>(glm::vec3(0.f, 1.0f, 0.f))
				.attachComponent<MeshComponent>(gltfScene.mMeshNodes[0].mMeshHandle)
				.attachComponent<MaterialComponent>(material)
				.attachComponent<BoundingBoxComponent>(gltfScene.mMeshNodes[0].mMin, gltfScene.mMeshNodes[0].mMax)
				.attachComponent<ForwardPBRRenderComponent>()
				.attachComponent<OpaqueComponent>()
				.attachComponent<ShadowCasterRenderComponent>()
			));
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

	void Demo::render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) {
		_tickParticles(resourceManagers, ecs);

		if (const auto& lightView = ecs.getSingleView<MainLightComponent, PointLightComponent, ShadowCameraComponent>()) {
			const auto& [lightEntity, _, __, shadowCamera] = *lightView;
			if (resourceManagers.mTextureManager.isValid(shadowCamera.mShadowMap)) {
				auto& shadowTexture = resourceManagers.mTextureManager.resolve(shadowCamera.mShadowMap);
				glViewport(0, 0, shadowTexture.mWidth, shadowTexture.mHeight);
				drawPointLightShadows<OpaqueComponent>(resourceManagers, ecs, lightEntity, true);
			}
		}

		const auto&& [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();
		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());

		auto sceneTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"Scene Target",
			FramebufferBuilder{}
			.setSize(viewport.mSize)
			.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGB16_UNORM })
			.attach(TextureFormat{ types::texture::Target::Texture2D,types::texture::InternalFormats::D16 }),
			resourceManagers.mTextureManager
		);
		if (!resourceManagers.mFramebufferManager.isValid(sceneTargetHandle)) {
			return;
		}
		auto& sceneTarget = resourceManagers.mFramebufferManager.resolve(sceneTargetHandle);
		sceneTarget.bind();
		sceneTarget.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);

		drawForwardPBR<OpaqueComponent>(resourceManagers, ecs, cameraEntity);
		_drawParticles(resourceManagers, ecs);

		FramebufferHandle bloomHandle = bloom(resourceManagers, viewport.mSize, sceneTarget.mTextures[0], mBloomParams);
		if (!resourceManagers.mFramebufferManager.isValid(bloomHandle)) {
			bloomHandle = sceneTargetHandle;
		}

		TextureHandle averageLuminance = NEO_INVALID_HANDLE;
		{
			auto previousHDRColorHandle = resourceManagers.mFramebufferManager.asyncLoad(
				"Previous HDR Color",
				FramebufferBuilder{}
				.setSize(viewport.mSize)
				.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGBA16_F }),
				resourceManagers.mTextureManager
			);
			if (resourceManagers.mFramebufferManager.isValid(previousHDRColorHandle)) {
				const auto& previousHDRColor = resourceManagers.mFramebufferManager.resolve(previousHDRColorHandle);
				averageLuminance = calculateAutoexposure(resourceManagers, ecs, previousHDRColor.mTextures[0], mAutoExposureParams);
				TRACY_GPUN("Blit Previous HDR Color");
				blit(resourceManagers, previousHDRColor, resourceManagers.mFramebufferManager.resolve(bloomHandle).mTextures[0], viewport.mSize);
			}
		}

		FramebufferHandle tonemappedHandle = tonemap(resourceManagers, viewport.mSize, resourceManagers.mFramebufferManager.resolve(bloomHandle).mTextures[0], averageLuminance);
		if (!resourceManagers.mFramebufferManager.isValid(tonemappedHandle)) {
			tonemappedHandle = bloomHandle;
		}

		backbuffer.bind();
		backbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color);
		drawFXAA(resourceManagers, viewport.mSize, resourceManagers.mFramebufferManager.resolve(tonemappedHandle).mTextures[0]);
		// Don't forget the depth. Because reasons.
		glBlitNamedFramebuffer(sceneTarget.mFBOID, backbuffer.mFBOID,
			0, 0, viewport.mSize.x, viewport.mSize.y,
			0, 0, viewport.mSize.x, viewport.mSize.y,
			GL_DEPTH_BUFFER_BIT,
			GL_NEAREST
		);
	}

	void Demo::destroy() {
	}
}
