#include "CSM/CSM.hpp"
#include "Engine/Engine.hpp"

#include "PerspectiveUpdateSystem.hpp"
#include "CSMFitting.hpp"
#include "CSMCameraComponent.hpp"
#include "CSMShadowRenderer.hpp"

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
namespace CSM {
	namespace {
		START_COMPONENT(SeenByMock);
		END_COMPONENT();

		START_COMPONENT(MockCameraComponent);
		END_COMPONENT();

		ECS::EntityBuilder _createCamera(std::string name, float fov, float near, float far, glm::vec3 pos) {
			return std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>(name)
				.attachComponent<SpatialComponent>(pos, glm::vec3(1.f))
				.attachComponent<CameraComponent>(near, far, CameraComponent::Perspective{ fov, 1.f })
			);
		}

		ECS::EntityBuilder _createLight(ResourceManagers& resourceManagers, glm::vec3 position) {
			SpatialComponent spatial(position, glm::vec3(1.f));
			spatial.setLookDir(glm::vec3(0.f, -0.5f, 0.7f));
			ShadowCameraComponent shadowCamera(types::texture::Target::Texture2D, 2048, resourceManagers.mTextureManager);
			LineMeshComponent lineMesh(resourceManagers.mMeshManager, glm::vec3(1.f, 0.f, 1.f));
			return std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Light")
				.attachComponent<LightComponent>(glm::vec3(1.f))
				.attachComponent<DirectionalLightComponent>()
				.attachComponent<SpatialComponent>(spatial)
				.attachComponent<MainLightComponent>()
				.attachComponent<CameraComponent>(-2.f, 2.f, CameraComponent::Orthographic{ glm::vec2(-4.f, 2.f), glm::vec2(0.1f, 5.f) })
				.attachComponent<FrustumComponent>()
				.attachComponent<FrustumFitReceiverComponent>()
				.attachComponent<LineMeshComponent>(lineMesh)
				.attachComponent<ShadowCameraComponent>(shadowCamera)
			);
		}
	}

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "CSM";
		return config;
	}

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {

		/* Game objects */
		ecs.submitEntity(std::move(_createCamera("main camera", 45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5))
			.attachComponent<CameraControllerComponent>(0.4f, 7.f)
			.attachComponent<MainCameraComponent>()
			.attachComponent<FrustumComponent>()
		));

		// Perspective camera
		{
			LineMeshComponent lineMesh(resourceManagers.mMeshManager, glm::vec3(0.f, 1.f, 1.f));
			ecs.submitEntity(std::move(_createCamera("mockCamera", 50.f, 0.1f, 5.f, glm::vec3(0.f, 2.f, -0.f))
				.attachComponent<MockCameraComponent>()
				.attachComponent<LineMeshComponent>(lineMesh)
				.attachComponent<FrustumComponent>()
				.attachComponent<FrustumFitSourceComponent>()
			));
		}

		// Ortho camera, shadow camera, light
		ecs.submitEntity(_createLight(resourceManagers, glm::vec3(10.f, 20.f, 0.f)));

		// CSM cameras
		{
			auto csmCameraProto = ECS::EntityBuilder{}
				.attachComponent<SpatialComponent>()
				.attachComponent<CameraComponent>(-2.f, 2.f, CameraComponent::Orthographic{ glm::vec2(-4.f, 2.f), glm::vec2(0.1f, 5.f) })
				.attachComponent<FrustumComponent>()
				;
			LineMeshComponent lineMesh0(resourceManagers.mMeshManager, glm::vec3(1.f, 0.f, 0.f));
			ecs.submitEntity(std::move(ECS::EntityBuilder(csmCameraProto)
				.attachComponent<CSMCamera0>()
				.attachComponent<TagComponent>("CSMCamera0")
				.attachComponent<LineMeshComponent>(lineMesh0)
			));
			LineMeshComponent lineMesh1(resourceManagers.mMeshManager, glm::vec3(0.f, 1.f, 0.f));
			ecs.submitEntity(std::move(ECS::EntityBuilder(csmCameraProto)
				.attachComponent<CSMCamera1>()
				.attachComponent<TagComponent>("CSMCamera1")
				.attachComponent<LineMeshComponent>(lineMesh1)
			));
			LineMeshComponent lineMesh2(resourceManagers.mMeshManager, glm::vec3(0.f, 0.f, 1.f));
			ecs.submitEntity(std::move(ECS::EntityBuilder(csmCameraProto)
				.attachComponent<CSMCamera2>()
				.attachComponent<TagComponent>("CSMCamera2")
				.attachComponent<LineMeshComponent>(lineMesh2)
			));
			LineMeshComponent lineMesh3(resourceManagers.mMeshManager, glm::vec3(1.f, 1.f, 0.f));
			ecs.submitEntity(std::move(ECS::EntityBuilder(csmCameraProto)
				.attachComponent<CSMCamera3>()
				.attachComponent<TagComponent>("CSMCamera3")
				.attachComponent<LineMeshComponent>(lineMesh3)
			));
		}

		// Renderable
		for (int i = 0; i < 50; i++) {
			auto meshHandle = util::genRandomBool() ? HashedString("cube") : HashedString("sphere");
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(util::genRandomVec3(), 1.f);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<MeshComponent>(meshHandle)
				.attachComponent<MaterialComponent>(material)
				.attachComponent<SpatialComponent>(glm::vec3(util::genRandom(-10.f, 10.f), util::genRandom(0.5f, 1.f), util::genRandom(-10.f, 10.f)), glm::vec3(0.5f))
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<PhongRenderComponent>()
				.attachComponent<ShadowCasterRenderComponent>()
				.attachComponent<WireframeRenderComponent>()
			));
		}

		/* Ground plane */
		{
			auto mesh = resourceManagers.mMeshManager.resolve(HashedString("quad"));
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(glm::vec3(0.7f), 1.f);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<MeshComponent>(HashedString("quad"))
				.attachComponent<MaterialComponent>(material)
				.attachComponent<SpatialComponent>(glm::vec3(0.f), glm::vec3(25.f, 25.f, 1.f), glm::vec3(-1.56f, 0, 0))
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<PhongRenderComponent>()
				.attachComponent<TagComponent>("Ground")
			));
		}

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>(); // Update camera
		ecs.addSystem<FrustumSystem>(); // Calculate original frusta bounds
		ecs.addSystem<FrustaFittingSystem>(); // Fit one frusta into another
		ecs.addSystem<CSMFitting>(); // Break scene frustum into slices and fit CSMCameraN to those slices
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
		const auto&& [lightEntity, ___, shadowCamera] = *ecs.getSingleView<MainLightComponent, ShadowCameraComponent>();

		if (resourceManagers.mTextureManager.isValid(shadowCamera.mShadowMap)) {
			auto& shadowTexture = resourceManagers.mTextureManager.resolve(shadowCamera.mShadowMap);
			glViewport(0, 0, shadowTexture.mWidth, shadowTexture.mHeight);
			//drawShadows(resourceManagers, ecs, lightEntity, true);
			drawCSMShadows(resourceManagers, ecs, lightEntity, true);
		}

		backbuffer.bind();
		backbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
		drawPhong(resourceManagers, ecs, cameraEntity);
		drawLines(resourceManagers, ecs, cameraEntity);

		// Draw wireframe for anything being seen by the mock camera
		drawWireframe<SeenByMock>(resourceManagers, ecs, cameraEntity);
	}
}
