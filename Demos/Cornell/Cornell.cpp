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
			ShadowCameraComponent shadowCamera(types::texture::Target::TextureCube, 512, resourceManagers.mTextureManager);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Light")
				.attachComponent<SpatialComponent>(glm::vec3(0.f, 1.f - util::EP * 3, 0.5f), glm::vec3(10.f))
				.attachComponent<MainLightComponent>()
				.attachComponent<LightComponent>(glm::vec3(1.f))
				.attachComponent<PointLightComponent>()
				.attachComponent<ShadowCameraComponent>(shadowCamera)
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

	void Demo::render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) {
		{
			PointLightShadowMapParameters params = {
				0.01f
			};
			if (auto lightView = ecs.getSingleView<MainLightComponent, PointLightComponent, ShadowCameraComponent>()) {
				auto&& [lightEntity, __, ___, shadowCamera] = lightView.value();
				if (resourceManagers.mTextureManager.isValid(shadowCamera.mShadowMap)) {
					auto& shadowTexture = resourceManagers.mTextureManager.resolve(shadowCamera.mShadowMap);
					glViewport(0, 0, shadowTexture.mWidth, shadowTexture.mHeight);
					drawPointLightShadows<OpaqueComponent>(resourceManagers, ecs, lightEntity, true, params);
				}
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

		if (resourceManagers.mFramebufferManager.isValid(sceneTargetHandle)) {
			auto& sceneTarget = resourceManagers.mFramebufferManager.resolve(sceneTargetHandle);
			sceneTarget.bind();
			sceneTarget.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
			glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
			drawForwardPBR<OpaqueComponent>(resourceManagers, ecs, cameraEntity);

			backbuffer.bind();
			backbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
			drawFXAA(resourceManagers, viewport.mSize, resourceManagers.mFramebufferManager.resolve(sceneTargetHandle).mTextures[0]);
			// Don't forget the depth. Because reasons.
			glBlitNamedFramebuffer(sceneTarget.mFBOID, backbuffer.mFBOID,
				0, 0, viewport.mSize.x, viewport.mSize.y,
				0, 0, viewport.mSize.x, viewport.mSize.y,
				GL_DEPTH_BUFFER_BIT,
				GL_NEAREST
			);
		}
	}

	void Demo::destroy() {
	}

}
