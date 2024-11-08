#include "Base/BaseDemo.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"
#include "ECS/Component/RenderingComponent/ForwardPBRRenderComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "Renderer/RenderingSystems/ForwardPBRRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"

#include "Loader/GLTFImporter.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"
#include "Renderer/RenderingSystems/Blitter.hpp"

using namespace neo;

namespace {

	void _drawPhong(
		FrameGraph& fg,
		const ResourceManagers& _resourceManagers, 
		const ECS& _ecs, 
		const Viewport& viewport, 
		const ECS::Entity cameraEntity, 
		FramebufferHandle outhandle
	) {
		auto shaderHandle = _resourceManagers.mShaderManager.asyncLoad("Phong Shader", 
			SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "model.vert"},
				{ types::shader::Stage::Fragment, "phong.frag" }
			}
		);

		ShaderDefines passDefines;

		UBO _ubo;
		_ubo["P"] = _ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj();
		const auto& cameraSpatial = _ecs.cGetComponent<SpatialComponent>(cameraEntity);
		_ubo["V"] = cameraSpatial->getView();
		_ubo["camPos"] = cameraSpatial->getPosition();

		auto&& [lightEntity, _lightLight, light, lightSpatial] = *_ecs.getSingleView<MainLightComponent, LightComponent, SpatialComponent>();
		_ubo["lightCol"] = light.mColor;

		MakeDefine(DIRECTIONAL_LIGHT);
		MakeDefine(POINT_LIGHT);
		if (_ecs.has<DirectionalLightComponent>(lightEntity)) {
			passDefines.set(DIRECTIONAL_LIGHT);
			_ubo["lightDir"] = -lightSpatial.getLookDir();
		}
		else if (_ecs.has<PointLightComponent>(lightEntity)) {
			passDefines.set(POINT_LIGHT);
			_ubo["lightPos"] = lightSpatial.getPosition();
			_ubo["lightRadiance"] = light.mIntensity;
		}
		else {
			NEO_FAIL("Phong light needs a directional or point light component");
		}

		fg.pass(outhandle, viewport, [pdef = std::move(passDefines), cameraEntity, shaderHandle, ubo = std::move(_ubo)](const ResourceManagers& resourceManagers, const ECS& ecs) mutable {
			TRACY_GPU();

			// No transparency sorting on the view, because I'm lazy, and this is stinky phong renderer
			const auto& view = ecs.getView<const MeshComponent, const MaterialComponent, const SpatialComponent>();
			for (auto entity : view) {
				// VFC
				if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
					if (!culled->isInView(ecs, entity, cameraEntity)) {
						continue;
					}
				}

				ShaderDefines drawDefines(pdef);

				const auto& material = view.get<const MaterialComponent>(entity);
				MakeDefine(ALBEDO_MAP);
				MakeDefine(NORMAL_MAP);

				if (resourceManagers.mTextureManager.isValid(material.mAlbedoMap)) {
					drawDefines.set(ALBEDO_MAP);
				}
				if (resourceManagers.mTextureManager.isValid(material.mNormalMap)) {
					drawDefines.set(NORMAL_MAP);
				}

				auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, drawDefines);
				resolvedShader.bind();

				resolvedShader.bindUniform("albedo", material.mAlbedoColor);
				if (resourceManagers.mTextureManager.isValid(material.mAlbedoMap)) {
					resolvedShader.bindTexture("albedoMap", resourceManagers.mTextureManager.resolve(material.mAlbedoMap));
				}

				if (resourceManagers.mTextureManager.isValid(material.mNormalMap)) {
					resolvedShader.bindTexture("normalMap", resourceManagers.mTextureManager.resolve(material.mNormalMap));
				}

				const auto& drawSpatial = view.get<const SpatialComponent>(entity);
				resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
				resolvedShader.bindUniform("N", drawSpatial.getNormalMatrix());

				for (const auto& [key, val] : ubo) {
					resolvedShader.bindUniform(key, val);
				}

				resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle).draw();
			}
		});
	}

}

namespace Base {

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "Base Demo";
		return config;
	}

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs, resourceManagers);
		{
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Camera")
				.attachComponent<SpatialComponent>(glm::vec3(0, 0.6f, 5), glm::vec3(1.f))
				.attachComponent<CameraComponent>(1.f, 100.f, CameraComponent::Perspective{ 45.f, 1.f })
				.attachComponent<CameraControllerComponent>(0.4f, 7.f)
				.attachComponent<MainCameraComponent>()
			));
		}

		{
			SpatialComponent spatial(glm::vec3(0.f, 2.f, 20.f), glm::vec3(100.f));
			spatial.setLookDir(glm::vec3(-0.7f, -0.6f, -0.32f));
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Light")
				.attachComponent<LightComponent>(glm::vec3(1.f), 5.f)
				.attachComponent<MainLightComponent>()
				.attachComponent<DirectionalLightComponent>()
				.attachComponent<SpatialComponent>(spatial)
			));
		}

		{
			GLTFImporter::Scene gltfScene = Loader::loadGltfScene(resourceManagers, "bunny.gltf");
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(1.f, 0.f, 1.f, 1.f);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Bunny")
				.attachComponent<SpatialComponent>(glm::vec3(2.f, 0.0f, -1.f), glm::vec3(1.5f))
				.attachComponent<RotationComponent>(glm::vec3(0.f, 1.0f, 0.f))
				.attachComponent<MeshComponent>(gltfScene.mMeshNodes[0].mMeshHandle)
				.attachComponent<BoundingBoxComponent>(gltfScene.mMeshNodes[0].mMin, gltfScene.mMeshNodes[0].mMax)
				.attachComponent<ForwardPBRRenderComponent>()
				.attachComponent<OpaqueComponent>()
				.attachComponent<MaterialComponent>(material)
			));
		}
		{
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(1.f, 1.f, 0.f, 1.f);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Icosahedron")
				.attachComponent<SpatialComponent>(glm::vec3(-2.f, 1.0f, -1.f), glm::vec3(1.5f))
				.attachComponent<RotationComponent>(glm::vec3(1.f, 0.0f, 0.f))
				.attachComponent<MeshComponent>(HashedString("icosahedron"))
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<ForwardPBRRenderComponent>()
				.attachComponent<OpaqueComponent>()
				.attachComponent<MaterialComponent>(material)
			));
		}
		for (int i = 0; i < 5; i++) {
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(util::genRandomVec3(0.3f, 1.f), 0.3f);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<SpatialComponent>(glm::vec3(0.f, 1.0f, -1.f * i), glm::vec3(0.75f))
				.attachComponent<MeshComponent>(HashedString("cube"))
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<ForwardPBRRenderComponent>()
				.attachComponent<TransparentComponent>()
				.attachComponent<MaterialComponent>(material)
			));
		}

		{
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(1.f);
			material.mAlbedoMap = resourceManagers.mTextureManager.asyncLoad("grid", TextureFiles{ {"grid.png"}, {} });
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Grid")
				.attachComponent<SpatialComponent>(glm::vec3(0.f), glm::vec3(15.f, 15.f, 1.f), glm::vec3(-util::PI / 2.f, 0.f, 0.f))
				.attachComponent<MeshComponent>(HashedString("quad"))
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f, -0.5f, -0.01f), glm::vec3(0.5f, 0.5f, 0.01f), true)
				.attachComponent<ForwardPBRRenderComponent>()
				.attachComponent<AlphaTestComponent>()
				.attachComponent<MaterialComponent>(material)
			));
		}

		ecs.addSystem<CameraControllerSystem>();
		ecs.addSystem<RotationSystem>();
	}

	void Demo::imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs, resourceManagers);
	}

	void Demo::update(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs, resourceManagers);
	}

	void Demo::render(FrameGraph& fg, const ResourceManagers& resourceManagers, const ECS& ecs, FramebufferHandle backbufferHandle) {
		const auto&& [cameraEntity, __, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();

		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
		auto sceneTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"Scene Target",
			FramebufferBuilder{}
			.setSize(viewport.mSize)
			.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGB16_UNORM })
			.attach(TextureFormat{ types::texture::Target::Texture2D,types::texture::InternalFormats::D16 }),
			resourceManagers.mTextureManager
		);
		fg.clear(sceneTargetHandle, glm::vec4(0.2f, 0.2f, 0.2f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);

		_drawPhong(fg, resourceManagers, ecs, Viewport(0, 0, viewport.mSize), cameraEntity, sceneTargetHandle);

		blit(fg, Viewport(0, 0, viewport.mSize), resourceManagers, sceneTargetHandle, backbufferHandle);
	}

	void Demo::destroy() {
	}
}
