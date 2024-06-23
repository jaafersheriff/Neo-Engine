#include "FrustaFitting/FrustaFitting.hpp"
#include "Engine/Engine.hpp"

#include "MockCameraComponent.hpp"
#include "PerspectiveUpdateSystem.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/ShadowCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/PhongRenderComponent.hpp"
#include "ECS/Component/RenderingComponent/ShadowCasterRenderComponent.hpp"
#include "ECS/Component/RenderingComponent/WireframeRenderComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustaFittingSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumCullingSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumToLineSystem.hpp"

#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/RenderingSystems/ShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/LineRenderer.hpp"
#include "Renderer/RenderingSystems/WireframeRenderer.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
namespace FrustaFitting {
	namespace {
		START_COMPONENT(SeenByMock);
		END_COMPONENT();

		struct Camera {
			ECS::Entity mEntity;
			Camera(std::string name, ECS& ecs, float fov, float near, float far, glm::vec3 pos) {
				mEntity = ecs.createEntity();
				ecs.addComponent<TagComponent>(mEntity, name);
				ecs.addComponent<SpatialComponent>(mEntity, pos, glm::vec3(1.f));
				ecs.addComponent<CameraComponent>(mEntity, near, far, CameraComponent::Perspective{ fov, 1.f });
			}
		};

		struct Light {
			Light(MeshManager& meshManager, ECS& ecs, glm::vec3 position) {
				// Light object
				auto lightEntity = ecs.createEntity();
				ecs.addComponent<TagComponent>(lightEntity, "Light");
				auto spatial = ecs.addComponent<SpatialComponent>(lightEntity, position, glm::vec3(1.f));
				spatial->setLookDir(glm::vec3(0.f, -0.5f, 0.7f));
				ecs.addComponent<LightComponent>(lightEntity, glm::vec3(1.f));
				ecs.addComponent<DirectionalLightComponent>(lightEntity);
				ecs.addComponent<MainLightComponent>(lightEntity);

				// Shadow camera object
				auto cameraObject = ecs.createEntity();
				ecs.addComponent<TagComponent>(cameraObject, "Light camera");
				ecs.addComponent<CameraComponent>(cameraObject, -2.f, 2.f, CameraComponent::Orthographic{ glm::vec2(-4.f, 2.f), glm::vec2(0.1f, 5.f) });
				ecs.addComponent<SpatialComponent>(cameraObject, position, glm::vec3(1.f));
				ecs.addComponent<FrustumComponent>(cameraObject);
				ecs.addComponent<FrustumFitReceiverComponent>(cameraObject);
				ecs.addComponent<LineMeshComponent>(cameraObject, meshManager, glm::vec3(1.f, 0.f, 1.f));
				ecs.addComponent<ShadowCameraComponent>(cameraObject);
			}
		};
	}

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "FrustaFitting";
		return config;
	}

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {

		/* Game objects */
		Camera sceneCamera("main camera", ecs, 45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5));
		ecs.addComponent<CameraControllerComponent>(sceneCamera.mEntity, 0.4f, 7.f);
		ecs.addComponent<MainCameraComponent>(sceneCamera.mEntity);
		ecs.addComponent<FrustumComponent>(sceneCamera.mEntity);

		// Perspective camera
		Camera mockCamera("mockCamera", ecs, 50.f, 0.1f, 5.f, glm::vec3(0.f, 2.f, -0.f));
		ecs.addComponent<MockCameraComponent>(mockCamera.mEntity);
		ecs.addComponent<LineMeshComponent>(mockCamera.mEntity, resourceManagers.mMeshManager, glm::vec3(0.f, 1.f, 1.f));
		ecs.addComponent<FrustumComponent>(mockCamera.mEntity);
		ecs.addComponent<FrustumFitSourceComponent>(mockCamera.mEntity);

		// Ortho camera, shadow camera, light
		Light light(resourceManagers.mMeshManager, ecs, glm::vec3(10.f, 20.f, 0.f));

		// Renderable
		for (int i = 0; i < 50; i++) {
			auto entity = ecs.createEntity();
			auto meshHandle = util::genRandomBool() ? HashedString("cube") : HashedString("sphere");
			ecs.addComponent<MeshComponent>(entity, meshHandle);
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(util::genRandom(-10.f, 10.f), util::genRandom(0.5f, 1.f), util::genRandom(-10.f, 10.f)), glm::vec3(0.5f));

			auto mesh = resourceManagers.mMeshManager.resolve(meshHandle);
			ecs.addComponent<BoundingBoxComponent>(entity, glm::vec3(-0.5f), glm::vec3(0.5f));
			auto material = ecs.addComponent<MaterialComponent>(entity);
			material->mAlbedoColor = glm::vec4(util::genRandomVec3(), 1.f);
			ecs.addComponent<PhongRenderComponent>(entity);
			ecs.addComponent<ShadowCasterRenderComponent>(entity);
			ecs.addComponent<WireframeRenderComponent>(entity);
		}

		/* Ground plane */
		{
			auto entity = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f), glm::vec3(25.f, 25.f, 1.f), glm::vec3(-1.56f, 0, 0));
			ecs.addComponent<MeshComponent>(entity, HashedString("quad"));
			auto mesh = resourceManagers.mMeshManager.resolve(HashedString("quad"));
			ecs.addComponent<BoundingBoxComponent>(entity, glm::vec3(-0.5f), glm::vec3(0.5f));
			auto material = ecs.addComponent<MaterialComponent>(entity);
			material->mAlbedoColor = glm::vec4(glm::vec3(0.7f), 1.f);
			ecs.addComponent<PhongRenderComponent>(entity);
			ecs.addComponent<TagComponent>(entity, "Ground");
		}

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>(); // Update camera
		ecs.addSystem<FrustumSystem>(); // Calculate original frusta bounds
		ecs.addSystem<FrustaFittingSystem>(); // Fit one frusta into another
		ecs.addSystem<FrustumToLineSystem>(); // Create line mesh
		ecs.addSystem<FrustumCullingSystem>();
		ecs.addSystem<PerspectiveUpdateSystem>(); // Update mock perspective camera
	}

	void Demo::update(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(resourceManagers);
		const auto&& [mockCameraEntity, _, __] = *ecs.getSingleView<MockCameraComponent, SpatialComponent>();
		const auto& view = ecs.getView<const WireframeRenderComponent, CameraCulledComponent>();
		for (auto entity : view) {
			auto& culled = view.get<CameraCulledComponent>(entity);
			if (culled.isInView(ecs, entity, mockCameraEntity)) {
				if (!ecs.has<SeenByMock>(entity)) {
					ecs.addComponent<SeenByMock>(entity);
				}
			}
			else if (ecs.has<SeenByMock>(entity)) {
				ecs.removeComponent<SeenByMock>(entity);
			}
		}
	}

	void Demo::render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) {
		const auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
		const auto&& [cameraEntity, _, __] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();

		auto shadowTexture = resourceManagers.mTextureManager.asyncLoad("Shadow map",
			TextureBuilder{}
				.setDimension(glm::u16vec3(2048, 2048, 0))
				.setFormat(TextureFormat{types::texture::Target::Texture2D, types::texture::InternalFormats::D16})
		);
		auto shadowMapHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"Shadow map",
			FramebufferExternal{ {shadowTexture} },
			resourceManagers.mTextureManager
		);

		if (resourceManagers.mFramebufferManager.isValid(shadowMapHandle)) {
			auto& shadowMap = resourceManagers.mFramebufferManager.resolve(shadowMapHandle);
			shadowMap.clear(glm::uvec4(0.f, 0.f, 0.f, 0.f), types::framebuffer::AttachmentBit::Depth);
			drawShadows(resourceManagers, shadowMap, ecs);
		}

		backbuffer.bind();
		backbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
		drawPhong(resourceManagers, ecs, cameraEntity, shadowTexture);
		drawLines(resourceManagers, ecs, cameraEntity);

		// Draw wireframe for anything being seen by the mock camera
		drawWireframe<SeenByMock>(resourceManagers, ecs, cameraEntity);
	}
}
