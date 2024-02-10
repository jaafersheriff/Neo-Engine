#include "PBR/PBR.hpp"

#include "GBufferComponent.hpp"
#include "GBufferRenderer.hpp"
#include "LightPassRenderer.hpp"

#include "Engine/Engine.hpp"
#include "Loader/Loader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/ShadowCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/SpatialComponent/SinTranslateComponent.hpp"
#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"
#include "ECS/Component/RenderingComponent/ShadowCasterShaderComponent.hpp"
#include "ECS/Component/RenderingComponent/WireframeShaderComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumCullingSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustaFittingSystem.hpp"
#include "ECS/Systems/TranslationSystems/SinTranslateSystem.hpp"

#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/RenderingSystems/ShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
namespace PBR {
	namespace {

		struct Camera {
			ECS::Entity mEntity;
			Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
				mEntity = ecs.createEntity();
				ecs.addComponent<TagComponent>(mEntity, "Camera");
				ecs.addComponent<SpatialComponent>(mEntity, pos, glm::vec3(1.f));
				ecs.addComponent<PerspectiveCameraComponent>(mEntity, near, far, fov);
				ecs.addComponent<CameraControllerComponent>(mEntity, ls, ms);
			}
		};

		void _createPointLights(ECS& ecs, const int count) {
			ecs.getView<PointLightComponent>().each([&ecs](auto entity, auto comp) {
				NEO_UNUSED(comp);
				ecs.removeEntity(entity);
			});

			for (int i = 0; i < count; i++) {
				glm::vec3 position(
					util::genRandom(-150.f, 200.f),
					util::genRandom(0.f, 85.f),
					util::genRandom(-100.f, 100.f)
				);
				glm::vec3 scale(util::genRandom(10.f, 40.f));
				const auto entity = ecs.createEntity();
				ecs.addComponent<LightComponent>(entity, util::genRandomVec3(0.3f, 1.f));
				ecs.addComponent<PointLightComponent>(entity);
				ecs.addComponent<SinTranslateComponent>(entity, glm::vec3(0.f, util::genRandom(0.f, 45.f), 0.f), position);
				ecs.addComponent<SpatialComponent>(entity, position, scale);
			}
		}
	}

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "PBR";
		return config;
	}

	void Demo::init(ECS& ecs) {

		/* Game objects */
		Camera camera(ecs, 45.f, 1.f, 50.f, glm::vec3(0, 0.6f, 5), 0.4f, 15.f);
		ecs.addComponent<MainCameraComponent>(camera.mEntity);
		ecs.addComponent<FrustumComponent>(camera.mEntity);
		ecs.addComponent<FrustumFitSourceComponent>(camera.mEntity);

		{
			auto lightEntity = ecs.createEntity();
			ecs.addComponent<TagComponent>(lightEntity, "Light");
			auto spat = ecs.addComponent<SpatialComponent>(lightEntity);
			spat->setLookDir(glm::normalize(glm::vec3(-0.28f, -0.96f, -0.06f)));
			ecs.addComponent<LightComponent>(lightEntity, glm::vec3(1.f));
			ecs.addComponent<MainLightComponent>(lightEntity);
			ecs.addComponent<DirectionalLightComponent>(lightEntity);
			ecs.addComponent<WireframeShaderComponent>(lightEntity);
			ecs.addComponent<MeshComponent>(lightEntity, Library::getMesh("cube").mMesh);
		}
		{
			auto shadowCam = ecs.createEntity();
			ecs.addComponent<TagComponent>(shadowCam, "Shadow Camera");
			ecs.addComponent<OrthoCameraComponent>(shadowCam, -1.f, 1000.f, -100.f, 100.f, -100.f, 100.f);
			ecs.addComponent<ShadowCameraComponent>(shadowCam);
			ecs.addComponent<FrustumComponent>(shadowCam);
			ecs.addComponent<SpatialComponent>(shadowCam);
			ecs.addComponent<FrustumFitReceiverComponent>(shadowCam, 0.5f);
		}

		// Renderable
		for (int i = 0; i < 50; i++) {
			auto mesh = util::genRandomBool() ? Library::getMesh("cube") : Library::getMesh("sphere");
			auto entity = ecs.createEntity();
			ecs.addComponent<MeshComponent>(entity, mesh.mMesh);
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(util::genRandom(-10.f, 10.f), util::genRandom(0.5f, 1.f), util::genRandom(-10.f, 10.f)), glm::vec3(0.5f));
			ecs.addComponent<BoundingBoxComponent>(entity, mesh);
			MaterialComponent material;
			material.mAmbient = glm::vec3(0.3f);
			material.mDiffuse = util::genRandomVec3();
			ecs.addComponent<MaterialComponent>(entity, material);
			ecs.addComponent<GBufferShaderComponent>(entity);
			ecs.addComponent<ShadowCasterShaderComponent>(entity);
		}

		/* Ground plane */
		{
			auto ground = ecs.createEntity();
			ecs.addComponent<MeshComponent>(ground, Library::getMesh("quad").mMesh);
			ecs.addComponent<SpatialComponent>(ground, glm::vec3(0.f, 0.f, 0.f), glm::vec3(50.f, 50.f, 1.f), glm::vec3(-1.56f, 0, 0));
			ecs.addComponent<BoundingBoxComponent>(ground, Library::getMesh("quad"));
			MaterialComponent material;
			material.mAmbient = glm::vec3(0.2f);
			material.mDiffuse = glm::vec3(0.7f);
			ecs.addComponent<MaterialComponent>(ground, material);
			ecs.addComponent<GBufferShaderComponent>(ground);
			ecs.addComponent<TagComponent>(ground, "Ground");
		}

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>();
		ecs.addSystem<SinTranslateSystem>();
		ecs.addSystem<FrustumSystem>();
		ecs.addSystem<FrustaFittingSystem>();
		ecs.addSystem<FrustumCullingSystem>();
	}

	void Demo::imGuiEditor(ECS& ecs) {
		NEO_UNUSED(ecs);
		ImGui::Checkbox("Shadows", &mDrawShadows);
	}

	void Demo::render(const ECS& ecs, Framebuffer& backbuffer) {
		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
		const auto&& [cameraEntity, _, __] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();
		auto sceneTarget = Library::getPooledFramebuffer({ viewport.mSize, {
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

		auto shadowMap = Library::getPooledFramebuffer({ glm::uvec2(4096, 4096), { 
			TextureFormat{
				TextureTarget::Texture2D,
				GL_DEPTH_COMPONENT16,
				GL_DEPTH_COMPONENT
			} 
		} }, "Shadow map");
		if (mDrawShadows) {
			shadowMap->clear(glm::uvec4(0.f, 0.f, 0.f, 0.f), GL_DEPTH_BUFFER_BIT);
			drawShadows(*shadowMap, ecs);
		}

		auto& gbuffer = createGBuffer(viewport.mSize);
		gbuffer.bind();
		gbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
		drawGBuffer(ecs, cameraEntity, {});

		auto lightResolve = Library::getPooledFramebuffer({ viewport.mSize, {
			TextureFormat{
				TextureTarget::Texture2D,
				GL_RGB16F,
				GL_RGB,
			}
		} }, "LightResolve");
		lightResolve->bind();
		lightResolve->clear(glm::vec4(0.f, 0.f, 0.f, 1.f), GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
		drawDirectionalLights(ecs, cameraEntity, gbuffer, shadowMap->mTextures[0]);

		// I'm lazy so I'm just going to do the final combine here
		{
			TRACY_GPUN("Final Combine");
			sceneTarget->bind();
			sceneTarget->clear(glm::vec4(0.f, 0.f, 0.f, 0.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
			auto* combineShader = Library::createSourceShader("FinalCombine", SourceShader::ConstructionArgs{
				{ ShaderStage::VERTEX, "quad.vert"},
				{ ShaderStage::FRAGMENT, "sponza/combine.frag" }
				});
			ShaderDefines defines;
			auto& resolvedShader = combineShader->getResolvedInstance(defines);
			resolvedShader.bind();

			resolvedShader.bindTexture("lightOutput", *lightResolve->mTextures[0]);

			Library::getMesh("quad").mMesh->draw();

			// Don't forget the depth. Because reasons.
			glBlitNamedFramebuffer(gbuffer.mFBOID, sceneTarget->mFBOID,
				0, 0, viewport.mSize.x, viewport.mSize.y,
				0, 0, viewport.mSize.x, viewport.mSize.y,
				GL_DEPTH_BUFFER_BIT,
				GL_NEAREST
			);
		}

		backbuffer.bind();
		backbuffer.clear(glm::vec4(0,0,0, 1.f), GL_COLOR_BUFFER_BIT);
		drawFXAA(backbuffer, *sceneTarget->mTextures[0]);
		// Don't forget the depth. Because reasons.
		glBlitNamedFramebuffer(sceneTarget->mFBOID, backbuffer.mFBOID,
			0, 0, viewport.mSize.x, viewport.mSize.y,
			0, 0, viewport.mSize.x, viewport.mSize.y,
			GL_DEPTH_BUFFER_BIT,
			GL_NEAREST
		);
	}
}
