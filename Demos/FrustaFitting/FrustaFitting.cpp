#include "FrustaFitting/FrustaFitting.hpp"
#include "Engine/Engine.hpp"

#include "MockCameraComponent.hpp"
#include "PerspectiveUpdateSystem.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CameraComponent/ShadowCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/PhongShaderComponent.hpp"
#include "ECS/Component/RenderingComponent/ShadowCasterShaderComponent.hpp"
#include "ECS/Component/RenderingComponent/WireframeShaderComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustaFittingSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumCullingSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumToLineSystem.hpp"

#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/RenderingSystems/ShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/LineRenderer.hpp"
#include "Renderer/RenderingSystems/WireframeRenderer.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
namespace FrustaFitting {
	namespace {
		struct SeenByMock : public Component {
			virtual std::string getName() const override {
				return "SeenByMock";
			}
		};

		struct Camera {
			ECS::Entity mEntity;
			Camera(std::string name, ECS& ecs, float fov, float near, float far, glm::vec3 pos) {
				mEntity = ecs.createEntity();
				ecs.addComponent<TagComponent>(mEntity, name);
				ecs.addComponent<SpatialComponent>(mEntity, pos, glm::vec3(1.f));
				ecs.addComponent<PerspectiveCameraComponent>(mEntity, near, far, fov);
			}
		};

		struct Light {
			Light(ECS& ecs, glm::vec3 position) {
				// Light object
				auto lightEntity = ecs.createEntity();
				ecs.addComponent<TagComponent>(lightEntity, "Light");
				auto spatial = ecs.addComponent<SpatialComponent>(lightEntity, position, glm::vec3(1.f));
				spatial->setLookDir(glm::vec3(0.f, -0.5f, 0.7f));
				ecs.addComponent<LightComponent>(lightEntity, glm::vec3(1.f));
				ecs.addComponent<DirectionalLightComponent>(lightEntity);
				ecs.addComponent<MainLightComponent>(lightEntity);

				// Shadow camera object
				auto cameraObject = ecs.createEntity();
				ecs.addComponent<TagComponent>(cameraObject, "Light camera");
				ecs.addComponent<OrthoCameraComponent>(cameraObject, -2.f, 2.f, -4.f, 2.f, 0.1f, 5.f);
				ecs.addComponent<SpatialComponent>(cameraObject, position, glm::vec3(1.f));
				ecs.addComponent<FrustumComponent>(cameraObject);
				ecs.addComponent<FrustumFitReceiverComponent>(cameraObject);
				ecs.addComponent<LineMeshComponent>(cameraObject, glm::vec3(1.f, 0.f, 1.f));
				ecs.addComponent<ShadowCameraComponent>(cameraObject);
			}
		};

		struct Renderable {
			ECS::Entity mEntity;

			Renderable(ECS& ecs, Mesh* mesh, glm::vec3 position = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f), glm::vec3 rotation = glm::vec3(0.f)) {
				mEntity = ecs.createEntity();
				ecs.addComponent<MeshComponent>(mEntity, mesh);
				ecs.addComponent<SpatialComponent>(mEntity, position, scale, rotation);
			}
		};
	}

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "FrustaFitting";
		return config;
	}

	void Demo::init(ECS& ecs) {

		/* Game objects */
		Camera sceneCamera("main camera", ecs, 45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5));
		ecs.addComponent<CameraControllerComponent>(sceneCamera.mEntity, 0.4f, 7.f);
		ecs.addComponent<MainCameraComponent>(sceneCamera.mEntity);
		ecs.addComponent<FrustumComponent>(sceneCamera.mEntity);

		// Perspective camera
		Camera mockCamera("mockCamera", ecs, 50.f, 0.1f, 5.f, glm::vec3(0.f, 2.f, -0.f));
		ecs.addComponent<MockCameraComponent>(mockCamera.mEntity);
		ecs.addComponent<LineMeshComponent>(mockCamera.mEntity, glm::vec3(0.f, 1.f, 1.f));
		ecs.addComponent<FrustumComponent>(mockCamera.mEntity);
		ecs.addComponent<FrustumFitSourceComponent>(mockCamera.mEntity);

		// Ortho camera, shadow camera, light
		Light light(ecs, glm::vec3(10.f, 20.f, 0.f));

		// Renderable
		for (int i = 0; i < 50; i++) {
			auto mesh = util::genRandomBool() ? Library::getMesh("cube") : Library::getMesh("sphere");
			Renderable sphere(ecs, mesh, glm::vec3(util::genRandom(-10.f, 10.f), util::genRandom(0.5f, 1.f), util::genRandom(-10.f, 10.f)), glm::vec3(0.5f));
			ecs.addComponent<BoundingBoxComponent>(sphere.mEntity, mesh->mMin, mesh->mMax);
			auto material = ecs.addComponent<MaterialComponent>(sphere.mEntity);
			material->mAlbedoColor = glm::vec4(util::genRandomVec3(), 1.f);
			ecs.addComponent<PhongShaderComponent>(sphere.mEntity);
			ecs.addComponent<ShadowCasterShaderComponent>(sphere.mEntity);
			ecs.addComponent<WireframeShaderComponent>(sphere.mEntity);
		}

		/* Ground plane */
		Renderable receiver(ecs, Library::getMesh("quad"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(50.f, 50.f, 1.f), glm::vec3(-1.56f, 0, 0));
		ecs.addComponent<BoundingBoxComponent>(receiver.mEntity, Library::getMesh("quad")->mMin, Library::getMesh("quad")->mMin);
		auto material = ecs.addComponent<MaterialComponent>(receiver.mEntity);
		material->mAlbedoColor = glm::vec4(glm::vec3(0.7f), 1.f);
		ecs.addComponent<PhongShaderComponent>(receiver.mEntity);
		ecs.addComponent<TagComponent>(receiver.mEntity, "Ground");

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>(); // Update camera
		ecs.addSystem<FrustumSystem>(); // Calculate original frusta bounds
		ecs.addSystem<FrustaFittingSystem>(); // Fit one frusta into another
		ecs.addSystem<FrustumToLineSystem>(); // Create line mesh
		ecs.addSystem<FrustumCullingSystem>();
		ecs.addSystem<PerspectiveUpdateSystem>(); // Update mock perspective camera
	}

	void Demo::update(ECS& ecs) {
		const auto&& [mockCameraEntity, _, __] = *ecs.getSingleView<MockCameraComponent, SpatialComponent>();
		const auto& view = ecs.getView<const WireframeShaderComponent, CameraCulledComponent>();
		for (auto entity : view) {
			auto& culled = view.get<CameraCulledComponent>(entity);
			if (culled.isInView(ecs, entity, mockCameraEntity)) {
				if (!ecs.has<SeenByMock>(entity)) {
					ecs.addComponent<SeenByMock>(entity);
				}
			}
			else if (ecs.has<SeenByMock>(entity)) {
				ecs.removeComponent<SeenByMock>(entity);
			}
		}
	}

	void Demo::render(const ECS& ecs, Framebuffer& backbuffer) {
		const auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
		const auto&& [cameraEntity, _, __] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();

		auto shadowMap = Library::getPooledFramebuffer({ glm::uvec2(2048, 2048), {
			TextureFormat {
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::D16,
				types::texture::BaseFormats::Depth
			}
		} }, "Shadow map");
		shadowMap->clear(glm::uvec4(0.f, 0.f, 0.f, 0.f), GL_DEPTH_BUFFER_BIT);
		drawShadows(*shadowMap, ecs);

		backbuffer.bind();
		backbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
		drawPhong(ecs, cameraEntity, shadowMap->mTextures[0]);
		drawLines(ecs, cameraEntity);

		// Draw wireframe for anything being seen by the mock camera
		drawWireframe<SeenByMock>(ecs, cameraEntity);
	}
}
