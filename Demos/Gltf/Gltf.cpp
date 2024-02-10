#include "Gltf/Gltf.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"

#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */

namespace Gltf {

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "Gltf Demo";
		return config;
	}

	void Demo::init(ECS& ecs) {

		/* Camera */
		{
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, "Camera");
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0, 0.6f, 5), glm::vec3(1.f));
			ecs.addComponent<PerspectiveCameraComponent>(entity, 1.f, 100.f, 45.f);
			ecs.addComponent<CameraControllerComponent>(entity, 0.4f, 7.f);
			ecs.addComponent<MainCameraComponent>(entity);
		}

		{
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, "Light");
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 2.f, 20.f));
			ecs.addComponent<LightComponent>(entity, glm::vec3(1.f));
			ecs.addComponent<MainLightComponent>(entity);
			ecs.addComponent<PointLightComponent>(entity, glm::vec3(0.1, 0.05, 0.003f));
		}


		Loader::GltfScene scene = Loader::loadGltfScene("Triangle/Triangle.gltf");
		for (auto& node : scene.mNodes) {
			auto entity = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 1.0f, 0.f));
			ecs.addComponent<MeshComponent>(entity, node.mMesh.mMesh);
			ecs.addComponent<BoundingBoxComponent>(entity, node.mMesh);
			ecs.addComponent<PhongShaderComponent>(entity);
			ecs.addComponent<OpaqueComponent>(entity);
			auto material = ecs.addComponent<MaterialComponent>(entity);
			material->mAmbient = glm::vec3(0.2f);
			material->mDiffuse = glm::vec3(1.f, 0.f, 1.f);
			material->mSpecular = glm::vec3(1.f);
			material->mShininess = 20.f;
		}

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>();

	}

	void Demo::imGuiEditor(ECS& ecs) {
		NEO_UNUSED(ecs);
	}

	void Demo::update(ECS& ecs) {
		NEO_UNUSED(ecs);
	}

	void Demo::render(const ECS& ecs, Framebuffer& backbuffer) {
		const auto&& [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();

		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
		auto sceneTarget = Library::getPooledFramebuffer(PooledFramebufferDetails{ viewport.mSize, {
			TextureFormat{
				TextureTarget::Texture2D,
				GL_RGB16,
				GL_RGB,
			},
			TextureFormat{
				TextureTarget::Texture2D,
				GL_DEPTH_COMPONENT16,
				GL_DEPTH_COMPONENT,
			}
		} }, "Scene target");

		glm::vec3 clearColor = getConfig().clearColor;

		sceneTarget->clear(glm::vec4(clearColor, 1.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
		drawPhong<OpaqueComponent>(ecs, cameraEntity);
		drawPhong<AlphaTestComponent>(ecs, cameraEntity);

		backbuffer.clear(glm::vec4(clearColor, 1.f), GL_COLOR_BUFFER_BIT);
		drawFXAA(backbuffer, *sceneTarget->mTextures[0]);
	   // Don't forget the depth. Because reasons.
	   glBlitNamedFramebuffer(sceneTarget->mFBOID, backbuffer.mFBOID,
		   0, 0, viewport.mSize.x, viewport.mSize.y,
		   0, 0, viewport.mSize.x, viewport.mSize.y,
		   GL_DEPTH_BUFFER_BIT,
		   GL_NEAREST
	   );

	}

	void Demo::destroy() {
	}
}
