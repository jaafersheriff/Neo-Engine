#include "Engine/Engine.hpp"

#include "NormalVisualizer.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
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
#include "ResourceManager/ResourceManagers.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
namespace NormalVisualizer {
	struct Camera {
		Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
			auto entity = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(entity, pos, glm::vec3(1.f));
			ecs.addComponent<CameraComponent>(entity, near, far, CameraComponent::Perspective{fov, 1.f});
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

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {
		/* Game objects */
		Camera camera(ecs, 45.f, 0.1f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);

		Light(ecs, glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f));

		{
			GLTFImporter::Scene gltfScene = Loader::loadGltfScene(resourceManagers, "bunny.gltf");
			const auto& bunnyNode = gltfScene.mMeshNodes[0];

			auto entity = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f), glm::vec3(1.f));
			ecs.addComponent<MeshComponent>(entity, bunnyNode.mMeshHandle);
			auto material = ecs.addComponent<MaterialComponent>(entity);
			material->mAlbedoColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.f);
			ecs.addComponent<PhongRenderComponent>(entity);
			ecs.addComponent<WireframeRenderComponent>(entity);
			ecs.addComponent<OpaqueComponent>(entity);
			ecs.addComponent<TagComponent>(entity, "bunny");
			ecs.addComponent<RotationComponent>(entity, glm::vec3(0.f, 0.5f, 0.f));
		}

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>();
		ecs.addSystem<RotationSystem>();
	}

	void Demo::render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) {
		const auto&& [cameraEntity, _, camera, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, CameraComponent, SpatialComponent>();
		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());

		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
		backbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);

		drawPhong<OpaqueComponent>(resourceManagers, ecs, cameraEntity);

		{
			auto normalShaderHandle = resourceManagers.mShaderManager.asyncLoad("NormalVisualizer", SourceShader::ConstructionArgs{
				{types::shader::Stage::Vertex, "normal.vert"},
				{types::shader::Stage::Geometry, "normal.geom"},
				{types::shader::Stage::Fragment, "normal.frag"}
			});
			if (!resourceManagers.mShaderManager.isValid(normalShaderHandle)) {
				return;
			}

			auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(normalShaderHandle, {});

			for (const auto&& [__, mesh, material, spatial] : ecs.getView<MeshComponent, MaterialComponent, SpatialComponent>().each()) {

				resolvedShader.bindUniform("magnitude", mMagnitude);

				/* Load PV */
				resolvedShader.bindUniform("P", camera.getProj());
				resolvedShader.bindUniform("V", cameraSpatial.getView());

				resolvedShader.bindUniform("M", spatial.getModelMatrix());
				resolvedShader.bindUniform("N", spatial.getNormalMatrix());

				/* DRAW */
				resourceManagers.mMeshManager.resolve(mesh.mMeshHandle).draw();
			}
		}
	}

	void Demo::imGuiEditor(ECS& ecs) {
		NEO_UNUSED(ecs);
		ImGui::SliderFloat("Magnitude", &mMagnitude, 0.f, 1.f);
	}
}
