#include "Cornell/Cornell.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"

#include "ECS/Component/RenderingComponent/PhongShaderComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"

#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */

namespace Cornell {
	namespace {
		inline void insertObject(ECS& ecs, std::string name, Mesh* mesh, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, glm::vec3 color) {
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, name);
			ecs.addComponent<MeshComponent>(entity, mesh);
			ecs.addComponent<SpatialComponent>(entity, position, scale, rotation);
			auto material = ecs.addComponent<MaterialComponent>(entity);
			material->mAlbedoColor = glm::vec4(color.x, color.y, color.z, 1.f);
			ecs.addComponent<PhongShaderComponent>(entity);
			ecs.addComponent<OpaqueComponent>(entity);
		}
	}

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "Cornell";
		config.clearColor = glm::vec3(0.f);
		return config;
	}

	void Demo::init(ECS& ecs) {

		/* Camera */
		{
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, "Camera");
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 0.5f, 2.25f), glm::vec3(1.f));
			ecs.addComponent<PerspectiveCameraComponent>(entity, 0.1f, 100.f, 45.f);
			ecs.addComponent<CameraControllerComponent>(entity, 0.4f, 7.f);
			ecs.addComponent<MainCameraComponent>(entity);
		}

		{
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, "Light");
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 1.f - util::EP * 4, 0.5f), glm::vec3(0.25f), glm::vec3(glm::radians(90.f), 0.f, 0.f));
			ecs.addComponent<MainLightComponent>(entity);
			ecs.addComponent<LightComponent>(entity, glm::vec3(1.f));
			ecs.addComponent<MeshComponent>(entity, Library::getMesh("quad"));
			auto material = ecs.addComponent<MaterialComponent>(entity);
			material->mAlbedoColor = glm::vec4(1.f);
			ecs.addComponent<PhongShaderComponent>(entity);
			ecs.addComponent<OpaqueComponent>(entity);
			ecs.addComponent<PointLightComponent>(entity, glm::vec3(3.0, 1.0, 5.0f));
		}

		insertObject(ecs, "backwall",  Library::getMesh("quad"), glm::vec3(0.f, 0.5f, 0.f), glm::vec3(1.f), glm::vec3(0.f), glm::vec3(1.f));
		insertObject(ecs, "leftwall",  Library::getMesh("quad"), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.f), glm::vec3(0.f, glm::radians(90.f), 0.f), glm::vec3(1.f, 0.f, 0.f));
		insertObject(ecs, "rightwall", Library::getMesh("quad"), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.f), glm::vec3(0.f, glm::radians(-90.f), 0.f), glm::vec3(0.f, 1.f, 0.f));
		insertObject(ecs, "floor", Library::getMesh("quad"), glm::vec3(0.f, 0.f, 0.5f), glm::vec3(1.f), glm::vec3(glm::radians(-90.f), 0.f, 0.f), glm::vec3(1.f));
		insertObject(ecs, "ceiling", Library::getMesh("quad"), glm::vec3(0.f, 1.0f, 0.5f), glm::vec3(1.f), glm::vec3(glm::radians(90.f), 0.f, 0.f), glm::vec3(1.f));
		insertObject(ecs, "box1", Library::getMesh("cube"), glm::vec3(-0.2f, 0.35f, 0.4f), glm::vec3(0.25f, 0.7f, 0.25f), glm::vec3(0.f, glm::radians(33.f), 0.f), glm::vec3(1.f));
		insertObject(ecs, "box2", Library::getMesh("cube"), glm::vec3(0.2f, 0.15f, 0.6f), glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0.f, glm::radians(-17.f), 0.f), glm::vec3(1.f));

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>();
	}

	void Demo::render(const ECS& ecs, Framebuffer& backbuffer) {
		const auto&& [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();

		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
		auto sceneTarget = Library::getPooledFramebuffer({ viewport.mSize, {
			TextureFormat {
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::RGB8,
				types::texture::BaseFormats::RGB
			},
			TextureFormat {
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::Depth16,
				types::texture::BaseFormats::Depth
			}
		} }, "Scene target");

		glm::vec3 clearColor = getConfig().clearColor;

		sceneTarget->bind();
		sceneTarget->clear(glm::vec4(clearColor, 1.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
		drawPhong<OpaqueComponent>(ecs, cameraEntity);

		backbuffer.clear(glm::vec4(clearColor, 1.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawFXAA(backbuffer, *sceneTarget->mTextures[0]);
	}

	void Demo::update(ECS& ecs) {
		NEO_UNUSED(ecs);
	}

	void Demo::destroy() {
	}

}
