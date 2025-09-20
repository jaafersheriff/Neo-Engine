#include "Cornell/Cornell.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"

#include "ECS/Component/RenderingComponent/ForwardPBRRenderComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"

#include "Renderer/RenderingSystems/ForwardPBRRenderer.hpp"
#include "Renderer/RenderingSystems/PointLightShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "Loader/MeshGenerator.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */

namespace Cornell {
	namespace {
		inline void insertObject(ECS& ecs, std::string name, MeshHandle meshHandle, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, glm::vec3 color) {
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(color.x, color.y, color.z, 1.f);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>(name)
				.attachComponent<MeshComponent>(meshHandle)
				.attachComponent<MaterialComponent>(material)
				.attachComponent<SpatialComponent>(position, scale, rotation)
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f), true)
				.attachComponent<ForwardPBRRenderComponent>()
				.attachComponent<OpaqueComponent>()
				.attachComponent<ShadowCasterRenderComponent>()
			));
		}
	}

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "Cornell";
		return config;
	}

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(resourceManagers);

		/* Camera */
		{
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Camera")
				.attachComponent<SpatialComponent>(glm::vec3(0.f, 0.5f, 2.25f), glm::vec3(1.f))
				.attachComponent<CameraComponent>(1.f, 100.f, CameraComponent::Perspective{ 45.f, 1.f })
				.attachComponent<CameraControllerComponent>(0.4f, 7.f)
				.attachComponent<MainCameraComponent>()
			));
		}

		{
			PointLightShadowMapComponent shadowMap(512, resourceManagers.mTextureManager);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Light")
				.attachComponent<SpatialComponent>(glm::vec3(0.f, 1.f - util::EP * 3, 0.5f), glm::vec3(10.f))
				.attachComponent<MainLightComponent>()
				.attachComponent<LightComponent>(glm::vec3(1.f))
				.attachComponent<PointLightComponent>()
				.attachComponent<PointLightShadowMapComponent>(shadowMap)
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f), false)
			));
		}

		HashedString quadMesh("quad");
		insertObject(ecs, "backwall",  quadMesh, glm::vec3(0.f, 0.5f, 0.f),    glm::vec3(1.f, 1.f, 0.05f), glm::vec3(0.f), glm::vec3(1.f));
		insertObject(ecs, "leftwall",  quadMesh, glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.f, 1.f, 0.05f), glm::vec3(0.f, glm::radians(90.f), 0.f), glm::vec3(1.f, 0.f, 0.f));
		insertObject(ecs, "rightwall", quadMesh, glm::vec3(0.5f, 0.5f, 0.5f),  glm::vec3(1.f, 1.f, 0.05f), glm::vec3(0.f, glm::radians(-90.f), 0.f), glm::vec3(0.f, 1.f, 0.f));
		insertObject(ecs, "floor",     quadMesh, glm::vec3(0.f, 0.f, 0.5f),    glm::vec3(1.f, 1.f, 0.05f), glm::vec3(glm::radians(-90.f), 0.f, 0.f), glm::vec3(1.f));
		insertObject(ecs, "ceiling",   quadMesh, glm::vec3(0.f, 1.0f, 0.5f),   glm::vec3(1.f, 1.f, 0.05f), glm::vec3(glm::radians(90.f), 0.f, 0.f), glm::vec3(1.f));
		insertObject(ecs, "box1",      HashedString("cube"), glm::vec3(-0.2f, 0.35f, 0.4f), glm::vec3(0.25f, 0.7f, 0.25f), glm::vec3(0.f, glm::radians(33.f), 0.f), glm::vec3(1.f));
		insertObject(ecs, "sphere", HashedString("sphere"), glm::vec3(0.2f, 0.15f, 0.6f), glm::vec3(0.3f), glm::vec3(0.f), glm::vec3(1.f));

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>();
	}

	void Demo::render(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs, const TextureHandle& outputColor, const TextureHandle& outputDepth) {
		{
			PointLightShadowMapParameters params = {
				0.01f
			};
			if (auto lightView = ecs.getSingleView<MainLightComponent, PointLightComponent, PointLightShadowMapComponent>()) {
				auto&& [lightEntity, __, ___, shadowCamera] = lightView.value();
				drawPointLightShadows<OpaqueComponent>(renderPasses, resourceManagers, ecs, lightEntity, true, params);
			}
		}

		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
		TextureHandle sceneColor = resourceManagers.mTextureManager.asyncLoad("Scene Color",
			TextureBuilder{}
			.setFormat(TextureFormat{ types::texture::Target::Texture2D,
				types::texture::InternalFormats::RGB16_UNORM,
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
		drawForwardPBR<OpaqueComponent>(renderPasses, sceneTargetHandle, viewport.mSize, cameraEntity);

		auto outputTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"FXAA Target",
			FramebufferExternalAttachments{
				FramebufferAttachment{outputColor},
			},
			resourceManagers.mTextureManager
		);
		renderPasses.clear(outputTargetHandle, types::framebuffer::AttachmentBit::Color, glm::vec4(0.f, 0.f, 0.f, 1.f), "Clear Output");
		{
			RenderState blitState;
			blitState.mDepthState = std::nullopt;
			renderPasses.renderPass(outputTargetHandle, viewport.mSize, blitState, [sceneColor](const ResourceManagers& resourceManagers, const ECS&) {
				drawFXAA(resourceManagers, sceneColor);
			}, "FXAA");
		}
	}

	void Demo::destroy() {
	}

}
