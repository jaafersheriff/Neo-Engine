#include "Engine/Engine.hpp"

#include "NormalVisualizer.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderingComponent/OpaqueComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "Renderer/RenderingSystems/WireframeRenderer.hpp"
#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/RenderingSystems/RenderPass.hpp"

#include "Loader/GLTFImporter.hpp"
#include "ResourceManager/ResourceManagers.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
namespace NormalVisualizer {
	namespace {
		ECS::EntityBuilder _createCamera(float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
			return std::move(ECS::EntityBuilder{}
				.attachComponent<SpatialComponent>(pos, glm::vec3(1.f))
				.attachComponent<CameraComponent>(near, far, CameraComponent::Perspective{ fov, 1.f })
				.attachComponent<CameraControllerComponent>(ls, ms)
				.attachComponent<MainCameraComponent>()
			);
		}

		ECS::EntityBuilder _createLight(glm::vec3 pos, glm::vec3 col) {
			return std::move(ECS::EntityBuilder{}
				.attachComponent<SpatialComponent>(pos)
				.attachComponent<LightComponent>(col)
				.attachComponent<MainLightComponent>()
				.attachComponent<DirectionalLightComponent>()
			);
		}
	}

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "NormalVisualizer";
		config.shaderDir = "shaders/normalvisualizer/";
		return config;
	}

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {
		/* Game objects */
		ecs.submitEntity(_createCamera(45.f, 0.1f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f));
		ecs.submitEntity(_createLight(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f)));

		{
			Loader::loadGltfScene(ecs, resourceManagers, "bunny.gltf", glm::mat4(1.f), [](ECS& ecs, const GLTFImporter::MeshNode& node) {
				MaterialComponent material = node.mMaterial;
				material.mAlbedoColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.f);
				ecs.submitEntity(std::move(ECS::EntityBuilder{}
					.attachComponent<SpatialComponent>(glm::vec3(0.f), glm::vec3(1.f))
					.attachComponent<MeshComponent>(node.mMeshHandle)
					.attachComponent<MaterialComponent>(material)
					.attachComponent<PhongRenderComponent>()
					.attachComponent<WireframeRenderComponent>()
					.attachComponent<OpaqueComponent>()
					.attachComponent<TagComponent>("bunny")
					.attachComponent<RotationComponent>(glm::vec3(0.f, 0.5f, 0.f))
				));
			});
		}

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>();
		ecs.addSystem<RotationSystem>();
	}

	void Demo::render(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs, const TextureHandle& outputColor, const TextureHandle& outputDepth) {

		auto outputTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"Output Target",
			FramebufferExternalAttachments{
				FramebufferAttachment{outputColor},
				FramebufferAttachment{outputDepth},
			},
			resourceManagers.mTextureManager
		);

		renderPasses.clear(outputTargetHandle, types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth, glm::vec4(0.f, 0.f, 0.f, 1.f));

		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
		renderPasses.renderPass(outputTargetHandle, viewport.mSize, [](const ResourceManagers& resourceManagers, const ECS& ecs) {
			const auto&& [cameraEntity, _, camera, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, CameraComponent, SpatialComponent>();
			drawPhong<OpaqueComponent>(resourceManagers, ecs, cameraEntity);
		});
		renderPasses.renderPass(outputTargetHandle, viewport.mSize, [this](const ResourceManagers& resourceManagers, const ECS& ecs) {
			const auto&& [cameraEntity, _, camera, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, CameraComponent, SpatialComponent>();
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
		});
	}

	void Demo::imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs, resourceManagers);
		ImGui::SliderFloat("Magnitude", &mMagnitude, 0.f, 1.f);
	}
}
