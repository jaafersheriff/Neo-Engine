#include "DrawStress.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
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
#include "ECS/Component/RenderingComponent/PhongShaderComponent.hpp"

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

	void Demo::init(ECS& ecs) {

		/* Camera */
		{
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, "Camera");
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0, 0.6f, 5), glm::vec3(1.f));
			ecs.addComponent<PerspectiveCameraComponent>(entity, 1.f, 1000.f, 45.f);
			ecs.addComponent<CameraControllerComponent>(entity, 0.4f, 7.f);
			ecs.addComponent<MainCameraComponent>(entity);
			ecs.addComponent<FrustumComponent>(entity);
		}

		{
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, "Light");
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 2.f, 20.f));
			ecs.addComponent<MainLightComponent>(entity);
			ecs.addComponent<LightComponent>(entity, glm::vec3(1.f));
			ecs.addComponent<PointLightComponent>(entity, glm::vec3(0.4, 0.01, 0.007));
		}

		/* Bunny object */
		for(int i = 0; i < 10000; i++) {
			auto cube = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(cube, glm::vec3(util::genRandom(-50.f, 50.f), util::genRandom(-10.f, 10.f), util::genRandom(-50.f, 50.f)), glm::vec3(util::genRandom(0.5f, 1.5f)), util::genRandomVec3(-util::PI, util::PI));
			ecs.addComponent<MeshComponent>(cube, Library::getMesh("cube").mMesh);
			ecs.addComponent<BoundingBoxComponent>(cube, Library::getMesh("cube"));
			ecs.addComponent<PhongShaderComponent>(cube);
			ecs.addComponent<OpaqueComponent>(cube);
			auto material = ecs.addComponent<MaterialComponent_DEPRECATED>(cube);
			material->mAmbient = glm::vec3(0.2f);
			material->mDiffuse = util::genRandomVec3();
		}

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>();
		ecs.addSystem<FrustumSystem>();
		ecs.addSystem<FrustumCullingSystem>();
	}

	void Demo::update(ECS& ecs) {
		NEO_UNUSED(ecs);
	}

	void Demo::render(const ECS& ecs, Framebuffer& backbuffer) {
		const auto&& [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();

		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
		glm::vec3 clearColor = getConfig().clearColor;

		backbuffer.bind();
		backbuffer.clear(glm::vec4(clearColor, 1.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
		drawPhong<OpaqueComponent>(ecs, cameraEntity);
	}

	void Demo::destroy() {
	}

}
