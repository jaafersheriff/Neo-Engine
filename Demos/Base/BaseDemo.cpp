#include "Base/BaseDemo.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"
#include "ECS/Component/RenderingComponent/ForwardPBRRenderComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "Renderer/RenderingSystems/ForwardPBRRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"

#include "Loader/GLTFImporter.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"

using namespace neo;

/* Game object definitions */

namespace Base {

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "Base Demo";
		return config;
	}

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs, resourceManagers);
		{
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, "Camera");
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0, 0.6f, 5), glm::vec3(1.f));
			ecs.addComponent<CameraComponent>(entity, 1.f, 100.f, CameraComponent::Perspective{ 45.f, 1.f });
			ecs.addComponent<CameraControllerComponent>(entity, 0.4f, 7.f);
			ecs.addComponent<MainCameraComponent>(entity);
		}

		{
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, "Light");
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 2.f, 20.f), glm::vec3(100.f));
			ecs.addComponent<LightComponent>(entity, glm::vec3(1.f), 1000.f);
			ecs.addComponent<MainLightComponent>(entity);
			ecs.addComponent<PointLightComponent>(entity);
		}

		{
			GLTFImporter::Scene gltfScene = Loader::loadGltfScene(resourceManagers, "bunny.gltf");
			auto bunny = ecs.createEntity();
			ecs.addComponent<TagComponent>(bunny, "Bunny");
			ecs.addComponent<SpatialComponent>(bunny, glm::vec3(2.f, 0.0f, -1.f), glm::vec3(1.5f));
			ecs.addComponent<RotationComponent>(bunny, glm::vec3(0.f, 1.0f, 0.f));
			ecs.addComponent<MeshComponent>(bunny, gltfScene.mMeshNodes[0].mMeshHandle);
			ecs.addComponent<BoundingBoxComponent>(bunny, gltfScene.mMeshNodes[0].mMin, gltfScene.mMeshNodes[0].mMax);
			ecs.addComponent<ForwardPBRRenderComponent>(bunny);
			ecs.addComponent<OpaqueComponent>(bunny);
			auto material = ecs.addComponent<MaterialComponent>(bunny);
			material->mAlbedoColor = glm::vec4(1.f, 0.f, 1.f, 1.f);
		}
		{
			auto icosahedron = ecs.createEntity();
			ecs.addComponent<TagComponent>(icosahedron, "Icosahedron");
			ecs.addComponent<SpatialComponent>(icosahedron, glm::vec3(-2.f, 1.0f, -1.f), glm::vec3(1.5f));
			ecs.addComponent<RotationComponent>(icosahedron, glm::vec3(1.f, 0.0f, 0.f));
			ecs.addComponent<MeshComponent>(icosahedron, HashedString("icosahedron"));
			ecs.addComponent<BoundingBoxComponent>(icosahedron, glm::vec3(-0.5f), glm::vec3(0.5f));
			ecs.addComponent<ForwardPBRRenderComponent>(icosahedron);
			ecs.addComponent<OpaqueComponent>(icosahedron);
			auto material = ecs.addComponent<MaterialComponent>(icosahedron);
			material->mAlbedoColor = glm::vec4(1.f, 1.f, 0.f, 1.f);
		}
		for (int i = 0; i < 5; i++) {
			auto cube = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(cube, glm::vec3(0.f, 1.0f, -1.f * i), glm::vec3(0.75f));
			ecs.addComponent<MeshComponent>(cube, HashedString("cube"));
			ecs.addComponent<BoundingBoxComponent>(cube, glm::vec3(-0.5f), glm::vec3(0.5f));
			ecs.addComponent<ForwardPBRRenderComponent>(cube);
			ecs.addComponent<TransparentComponent>(cube);
			auto material = ecs.addComponent<MaterialComponent>(cube);
			material->mAlbedoColor = glm::vec4(util::genRandomVec3(0.3f, 1.f), 0.3f);
		}

		{
			auto plane = ecs.createEntity();
			ecs.addComponent<TagComponent>(plane, "Grid");
			ecs.addComponent<SpatialComponent>(plane, glm::vec3(0.f), glm::vec3(15.f, 15.f, 1.f), glm::vec3(-util::PI / 2.f, 0.f, 0.f));
			ecs.addComponent<MeshComponent>(plane, HashedString("quad"));
			ecs.addComponent<BoundingBoxComponent>(plane, glm::vec3(-0.5f, -0.5f, -0.01f), glm::vec3(0.5f, 0.5f, 0.01f), true);
			ecs.addComponent<ForwardPBRRenderComponent>(plane);
			ecs.addComponent<AlphaTestComponent>(plane);
			auto material = ecs.addComponent<MaterialComponent>(plane);
			material->mAlbedoColor = glm::vec4(1.f);
			material->mAlbedoMap = resourceManagers.mTextureManager.asyncLoad("grid", TextureFiles{ {"grid.png"}, {} });
		}

		ecs.addSystem<CameraControllerSystem>();
		ecs.addSystem<RotationSystem>();
	}

	void Demo::imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs, resourceManagers);
	}

	void Demo::update(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs, resourceManagers);
	}

	void Demo::render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) {
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
			sceneTarget.clear(glm::vec4(0.2f, 0.2f, 0.2f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
			glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
			drawForwardPBR<OpaqueComponent>(resourceManagers, ecs, cameraEntity);
			drawForwardPBR<AlphaTestComponent>(resourceManagers, ecs, cameraEntity);
			drawForwardPBR<TransparentComponent>(resourceManagers, ecs, cameraEntity);

			backbuffer.bind();
			backbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color);
			drawFXAA(resourceManagers, viewport.mSize, sceneTarget.mTextures[0]);
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
