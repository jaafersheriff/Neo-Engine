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

#include <entt/graph/flow.hpp>

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
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Camera")
				.attachComponent<SpatialComponent>(glm::vec3(0, 0.6f, 5), glm::vec3(1.f))
				.attachComponent<CameraComponent>(1.f, 100.f, CameraComponent::Perspective{ 45.f, 1.f })
				.attachComponent<CameraControllerComponent>(0.4f, 7.f)
				.attachComponent<MainCameraComponent>()
			));
		}

		{
			SpatialComponent spatial(glm::vec3(0.f, 2.f, 20.f), glm::vec3(100.f));
			spatial.setLookDir(glm::vec3(-0.7f, -0.6f, -0.32f));
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Light")
				.attachComponent<LightComponent>(glm::vec3(1.f), 5.f)
				.attachComponent<MainLightComponent>()
				.attachComponent<DirectionalLightComponent>()
				.attachComponent<SpatialComponent>(spatial)
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
				.attachComponent<BoundingBoxComponent>(gltfScene.mMeshNodes[0].mMin, gltfScene.mMeshNodes[0].mMax)
				.attachComponent<ForwardPBRRenderComponent>()
				.attachComponent<OpaqueComponent>()
				.attachComponent<MaterialComponent>(material)
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
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<ForwardPBRRenderComponent>()
				.attachComponent<OpaqueComponent>()
				.attachComponent<MaterialComponent>(material)
			));
		}
		for (int i = 0; i < 5; i++) {
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(util::genRandomVec3(0.3f, 1.f), 0.3f);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<SpatialComponent>(glm::vec3(0.f, 1.0f, -1.f * i), glm::vec3(0.75f))
				.attachComponent<MeshComponent>(HashedString("cube"))
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<ForwardPBRRenderComponent>()
				.attachComponent<TransparentComponent>()
				.attachComponent<MaterialComponent>(material)
			));
		}

		{
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(1.f);
			material.mAlbedoMap = resourceManagers.mTextureManager.asyncLoad("grid", TextureFiles{ {"grid.png"}, {} });
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Grid")
				.attachComponent<SpatialComponent>(glm::vec3(0.f), glm::vec3(15.f, 15.f, 1.f), glm::vec3(-util::PI / 2.f, 0.f, 0.f))
				.attachComponent<MeshComponent>(HashedString("quad"))
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f, -0.5f, -0.01f), glm::vec3(0.5f, 0.5f, 0.01f), true)
				.attachComponent<ForwardPBRRenderComponent>()
				.attachComponent<AlphaTestComponent>()
				.attachComponent<MaterialComponent>(material)
			));
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
		entt::flow builder{};
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
			builder.bind(HashedString("clear - sceneTarget"))
				.rw(sceneTargetHandle.mHandle);

			glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
			drawForwardPBR<OpaqueComponent>(resourceManagers, ecs, cameraEntity);
			builder.bind(HashedString("drawForwardPBR - Opaque"))
				.rw(sceneTargetHandle.mHandle);
			drawForwardPBR<AlphaTestComponent>(resourceManagers, ecs, cameraEntity);
			builder.bind(HashedString("drawForwardPBR - AlphaTest"))
				.rw(sceneTargetHandle.mHandle);
			drawForwardPBR<TransparentComponent>(resourceManagers, ecs, cameraEntity);
			builder.bind(HashedString("drawForwardPBR - Transparent"))
				.rw(sceneTargetHandle.mHandle);

			backbuffer.bind();
			backbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color);
			builder.bind(HashedString("clear - backbuffer"))
				.rw(HashedString("Backbuffer"));
			drawFXAA(resourceManagers, viewport.mSize, sceneTarget.mTextures[0]);
			builder.bind(HashedString("fxaa - sceneTarget --> backbuffer"))
				.ro(sceneTargetHandle.mHandle)
				.rw(HashedString("Backbuffer"));
			// Don't forget the depth. Because reasons.
			glBlitNamedFramebuffer(sceneTarget.mFBOID, backbuffer.mFBOID,
				0, 0, viewport.mSize.x, viewport.mSize.y,
				0, 0, viewport.mSize.x, viewport.mSize.y,
				GL_DEPTH_BUFFER_BIT,
				GL_NEAREST
			);
			builder.bind(HashedString("depth blit - sceneTarget --> backbuffer"))
				.ro(sceneTargetHandle.mHandle)
				.rw(HashedString("Backbuffer"));
		}

		auto graph = builder.graph();
		std::ostringstream output{};
		//entt::dot(output, graph);
		entt::dot(output, graph, [&](auto& output, auto vertex) {
			//auto node1 = graph[vertex];
			auto node2 = builder[vertex];
			output << "label=\"v" << node2 << "\",shape=\"box\"";
		});
		printf("%s\n", output.str().c_str());
	}

	void Demo::destroy() {
	}
}
