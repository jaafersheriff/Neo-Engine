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
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */

namespace Gltf {
	template<typename... CompTs>
	void _drawGltf(const ECS& ecs, ECS::Entity cameraEntity) {
		TRACY_GPU();

		ShaderDefines passDefines({});
		bool containsAlphaTest = false;
		MakeDefine(ALPHA_TEST);
		if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
			containsAlphaTest = true;
			passDefines.set(ALPHA_TEST);
		}

		const glm::mat4 P = ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj();
		const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
		auto&& [lightEntity, _lightLight, light, lightSpatial] = *ecs.getSingleView<MainLightComponent, LightComponent, SpatialComponent>();

		bool directionalLight = ecs.has<DirectionalLightComponent>(lightEntity);
		bool pointLight = ecs.has<PointLightComponent>(lightEntity);
		glm::vec3 attenuation(0.f);
		MakeDefine(DIRECTIONAL_LIGHT);
		MakeDefine(POINT_LIGHT);
		if (directionalLight) {
			passDefines.set(DIRECTIONAL_LIGHT);
		}
		else if (pointLight) {
			attenuation = ecs.cGetComponent<PointLightComponent>(lightEntity)->mAttenuation;
			passDefines.set(POINT_LIGHT);
		}
		else {
			NEO_FAIL("Phong light needs a directional or point light component");
		}

		SourceShader* shader = Library::createSourceShader("GltfShader", SourceShader::ConstructionArgs{
				{ ShaderStage::VERTEX, "model.vert"},
				{ ShaderStage::FRAGMENT, "gltf.frag" }
		});

		ShaderDefines drawDefines(passDefines);
		const auto& view = ecs.getView<const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
		for (auto entity : view) {
			// VFC
			if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
				if (!culled->isInView(ecs, entity, cameraEntity)) {
					continue;
				}
			}

			if (containsAlphaTest) {
				NEO_ASSERT(!ecs.has<OpaqueComponent>(entity), "Entity has opaque and alpha test component?");
			}

			drawDefines.reset();
			const auto& material = view.get<const MaterialComponent>(entity);

			auto& resolvedShader = shader->getResolvedInstance(drawDefines);
			resolvedShader.bind();

			resolvedShader.bindUniform("albedo", material.mAlbedoColor);

			// UBO candidates
			{
				resolvedShader.bindUniform("P", P);
				resolvedShader.bindUniform("V", cameraSpatial->getView());
				resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());
				resolvedShader.bindUniform("lightCol", light.mColor);
				if (directionalLight) {
					resolvedShader.bindUniform("lightDir", -lightSpatial.getLookDir());
				}
				if (pointLight) {
					resolvedShader.bindUniform("lightPos", lightSpatial.getPosition());
					resolvedShader.bindUniform("lightAtt", attenuation);
				}
			}

			const auto& drawSpatial = view.get<const SpatialComponent>(entity);
			resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
			resolvedShader.bindUniform("N", drawSpatial.getNormalMatrix());

			view.get<const MeshComponent>(entity).mMesh->draw();
		}
	}

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


		Loader::GltfScene scene = Loader::loadGltfScene("SimpleTexture/SimpleTexture.gltf");
		for (auto& node : scene.mMeshNodes) {
			auto entity = ecs.createEntity();
			if (!node.mName.empty()) {
				ecs.addComponent<TagComponent>(entity, node.mName);
			}
			ecs.addComponent<SpatialComponent>(entity, node.mSpatial);
			ecs.addComponent<MeshComponent>(entity, node.mMesh.mMesh);
			ecs.addComponent<BoundingBoxComponent>(entity, node.mMesh);
			if (node.mAlphaMode == Loader::GltfScene::MeshNode::AlphaMode::Opaque) {
				ecs.addComponent<OpaqueComponent>(entity);
			}
			else if (node.mAlphaMode == Loader::GltfScene::MeshNode::AlphaMode::AlphaTest) {
				ecs.addComponent<AlphaTestComponent>(entity);
			}
			ecs.addComponent<MaterialComponent>(entity, node.mMaterial);
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
		_drawGltf<OpaqueComponent>(ecs, cameraEntity);
		_drawGltf<AlphaTestComponent>(ecs, cameraEntity);

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
