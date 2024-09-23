#include "SPD.hpp"
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

using namespace neo;

/* Game object definitions */

namespace SPD {

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "SPD Demo";
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

		for(int i = 0; i < 20; i++) {
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(util::genRandomVec3(), 1.f);
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<SpatialComponent>(util::genRandomVec3(-8.f, 8.f), glm::vec3(util::genRandom(1.f, 3.f)))
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

	void _renderScene() {

	}

	void Demo::render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) {
		const auto&& [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();
		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());

		backbuffer.bind();
		backbuffer.clear(glm::vec4(0.0f, 0.0f, 0.0f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);

		// I'm lazy
		{
			TRACY_GPUN("Occlusion Test");

			auto occlusionTestHandle = resourceManagers.mShaderManager.asyncLoad("Occlusion Test", SourceShader::ConstructionArgs{
				{types::shader::Stage::Vertex, "spd/spd_model.vert"},
				{types::shader::Stage::Fragment, "spd/occlusiontest.frag"},
				});
			if (!resourceManagers.mShaderManager.isValid(occlusionTestHandle)) {
				return;
			}
			ShaderDefines drawDefines;
			MakeDefine(BOUNDING_BOX);

			glDepthFunc(GL_ALWAYS);

			const auto& view = ecs.getView<const MeshComponent, const MaterialComponent, const SpatialComponent>();
			for (auto entity : view) {
				// VFC
				if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
					if (!culled->isInView(ecs, entity, cameraEntity)) {
						continue;
					}
				}
				if (ecs.has<TransparentComponent>(entity)) {
					continue;
				}

				glm::vec3 bbMin(0.f);
				glm::vec3 bbMax(0.f);
				drawDefines.reset();
				if (auto* bb = ecs.cGetComponent<BoundingBoxComponent>(entity)) {
					drawDefines.set(BOUNDING_BOX);
					bbMin = bb->mMin;
					bbMax = bb->mMax;
				}
				auto& occlusionShader = resourceManagers.mShaderManager.resolveDefines(occlusionTestHandle, drawDefines);

				occlusionShader.bindUniform("P", ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj());
				occlusionShader.bindUniform("V", cameraSpatial.getView());

				occlusionShader.bindUniform("bbMin", bbMin);
				occlusionShader.bindUniform("bbMax", bbMax);
				if (resourceManagers.mTextureManager.isValid(mHiZTextureHandle)) {
					auto& hiZTexture = resourceManagers.mTextureManager.resolve(mHiZTextureHandle);
					occlusionShader.bindTexture("hiZ", hiZTexture);
					occlusionShader.bindUniform("hiZMips", hiZTexture.mFormat.mMipCount);
					occlusionShader.bindUniform("hiZDimension", glm::vec2(hiZTexture.mWidth, hiZTexture.mHeight));
				}
				else {
					auto& hiZTexture = resourceManagers.mTextureManager.resolve("white");
					occlusionShader.bindTexture("hiZ", hiZTexture);
					occlusionShader.bindUniform("hiZMips", 1);
					occlusionShader.bindUniform("hiZDimension", glm::vec2(1, 1));
				}

				const auto& drawSpatial = view.get<const SpatialComponent>(entity);
				occlusionShader.bindUniform("M", drawSpatial.getModelMatrix());
				resourceManagers.mMeshManager.resolve(ecs.cGetComponent<MeshComponent>(entity)->mMeshHandle).draw();
			}
			glDepthFunc(GL_LESS);
		}

		// TODO - this needs ro be generated from last frame and then reused for occlusion test
		mHiZTextureHandle = downSample(backbuffer.mTextures[1], resourceManagers);
	}

	void Demo::destroy() {
	}
}
