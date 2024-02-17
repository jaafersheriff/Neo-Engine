#include "Engine/Engine.hpp"

#include "NormalVisualizer.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "Renderer/RenderingSystems/WireframeRenderer.hpp"
#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "Loader/GLTFImporter.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
namespace NormalVisualizer {
	struct Camera {
		Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
			auto entity = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(entity, pos, glm::vec3(1.f));
			ecs.addComponent<PerspectiveCameraComponent>(entity, near, far, fov);
			ecs.addComponent<CameraControllerComponent>(entity, ls, ms);
			ecs.addComponent<MainCameraComponent>(entity);
		}
	};

	struct Light {

		Light(ECS& ecs, glm::vec3 pos, glm::vec3 col) {
			auto entity = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(entity, pos);
			ecs.addComponent<LightComponent>(entity, col);
			ecs.addComponent<MainLightComponent>(entity);
			ecs.addComponent<DirectionalLightComponent>(entity);
		}
	};

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "NormalVisualizer";
		config.shaderDir = "shaders/normalvisualizer/";
		config.clearColor = { 0.2f, 0.3f, 0.4f };
		return config;
	}

	void Demo::init(ECS& ecs) {
		/* Game objects */
		Camera camera(ecs, 45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);

		Light(ecs, glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f));

		{
			GLTFImporter::Scene gltfScene = Loader::loadGltfScene("bunny.gltf");
			const auto& bunnyNode = gltfScene.mMeshNodes[0];

			auto entity = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f), glm::vec3(1.f));
			ecs.addComponent<RotationComponent>(entity, glm::vec3(0.f, 0.6f, 0.f));
			ecs.addComponent<MeshComponent>(entity, bunnyNode.mMesh.mMesh);
			ecs.addComponent<MaterialComponent_DEPRECATED>(entity);
			ecs.addComponent<PhongShaderComponent>(entity);
			ecs.addComponent<WireframeShaderComponent>(entity);
			ecs.addComponent<OpaqueComponent>(entity);
			ecs.addComponent<TagComponent>(entity, "bunny");
		}

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>();
		ecs.addSystem<RotationSystem>();
	}

	void Demo::render(const ECS& ecs, Framebuffer& backbuffer) {
		const auto&& [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();

		backbuffer.bind();
		backbuffer.clear(glm::vec4(getConfig().clearColor, 1.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drawPhong<OpaqueComponent>(ecs, cameraEntity);
		drawWireframe(ecs, cameraEntity);

		{
			auto& normalShader = Library::createSourceShader("NormalVisualizer", SourceShader::ConstructionArgs{
				{ShaderStage::VERTEX, "normal.vert"},
				{ShaderStage::GEOMETRY, "normal.geom"},
				{ShaderStage::FRAGMENT, "normal.frag"}
				})->getResolvedInstance({});
				normalShader.bind();

				normalShader.bindUniform("magnitude", mMagnitude);

				/* Load PV */
				normalShader.bindUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
				normalShader.bindUniform("V", cameraSpatial.getView());

				for (const auto&& [__, mesh, spatial] : ecs.getView<MeshComponent, SpatialComponent>().each()) {
					normalShader.bindUniform("M", spatial.getModelMatrix());
					normalShader.bindUniform("N", spatial.getNormalMatrix());

					/* DRAW */
					mesh.mMesh->draw();
				}
		}
	}

	void Demo::imGuiEditor(ECS& ecs) {
		NEO_UNUSED(ecs);
		ImGui::SliderFloat("Magnitude", &mMagnitude, 0.f, 1.f);
	}
}
