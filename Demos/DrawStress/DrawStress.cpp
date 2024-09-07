#include "DrawStress.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"
#include "ECS/Component/RenderingComponent/OpaqueComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"
#include "ECS/Component/RenderingComponent/PhongRenderComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */

namespace DrawStress {

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "DrawStress";
		return config;
	}

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(resourceManagers);

		/* Camera */
		{
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Camera")
				.attachComponent<SpatialComponent>(glm::vec3(0, 0.6f, 5), glm::vec3(1.f))
				.attachComponent<CameraComponent>(1.f, 100.f, CameraComponent::Perspective{ 45.f, 1.f })
				.attachComponent<CameraControllerComponent>(0.4f, 7.f)
				.attachComponent<MainCameraComponent>()
				.attachComponent<FrustumComponent>()
			));
		}

		{
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Light")
				.attachComponent<SpatialComponent>(glm::vec3(0.f, 2.f, 20.f))
				.attachComponent<MainLightComponent>()
				.attachComponent<LightComponent>(glm::vec3(1.f), 15.f)
				.attachComponent<PointLightComponent>()
			));
		}

		/* Bunny object */
		for(int i = 0; i < 10000; i++) {
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(util::genRandomVec3(), 1.f);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<SpatialComponent>(glm::vec3(util::genRandom(-50.f, 50.f), util::genRandom(-10.f, 10.f), util::genRandom(-50.f, 50.f)), glm::vec3(util::genRandom(0.5f, 1.5f)), util::genRandomVec3(-util::PI, util::PI))
				.attachComponent<MeshComponent>(HashedString("cube"))
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<PhongRenderComponent>()
				.attachComponent<OpaqueComponent>()
				.attachComponent<MaterialComponent>(material)
			));
		}

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>();
		ecs.addSystem<FrustumSystem>();
		ecs.addSystem<FrustumCullingSystem>();
	}

	void Demo::render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) {
		const auto&& [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();

		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());

		backbuffer.bind();
		backbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
		drawPhong<OpaqueComponent>(resourceManagers, ecs, cameraEntity);
	}

	void Demo::destroy() {
	}

}
