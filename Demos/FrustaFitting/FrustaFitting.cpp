#include "FrustaFitting/FrustaFitting.hpp"
#include "Engine/Engine.hpp"

#include "MockCameraComponent.hpp"
#include "PerspectiveUpdateSystem.hpp"

#include "Renderer/Shader/ShadowCasterShader.hpp"
#include "Renderer/Shader/LineShader.hpp"
#include "Renderer/Shader/WireFrameShader.hpp"
#include "Renderer/Shader/PhongShadowShader.hpp"
#include "WireShader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustaFittingSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumToLineSystem.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

static constexpr int shadowMapSize = 2048;

/* Game object definitions */
namespace FrustaFitting {

    struct Camera {
        ECS::Entity mEntity;
        Camera(std::string name, ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
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
            ecs.addComponent<LightComponent>(lightEntity, glm::vec3(1.f), glm::vec3(0.4f, 0.2f, 0.f));

            // Shadow camera object
            auto cameraObject = ecs.createEntity();
            ecs.addComponent<TagComponent>(lightEntity, "Light camera");
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
            ecs.addComponent<MeshComponent>(mEntity, *mesh);
            ecs.addComponent<SpatialComponent>(mEntity, position, scale, rotation);
        }
    };


    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "FrustaFitting";
        return config;
    }

    void Demo::init(ECS& ecs, Renderer& renderer) {

        /* Game objects */
        Camera sceneCamera("main camera", ecs, 45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5));
        ecs.addComponent<CameraControllerComponent>(sceneCamera.mEntity, 0.4f, 7.f);
        ecs.addComponent<MainCameraComponent>(sceneCamera.mEntity);

        // Perspective camera
        Camera mockCamera("mockCamera", ecs, 50.f, 0.1f, 5.f, glm::vec3(0.f, 2.f, -0.f));
        ecs.addComponent<MockCameraComponent>(mockCamera.mEntity);
        ecs.addComponent<LineMeshComponent>(mockCamera.mEntity, glm::vec3(0.f, 1.f, 1.f));
        ecs.addComponent<FrustumComponent>(mockCamera.mEntity);
        ecs.addComponent<FrustumFitSourceComponent>(mockCamera.mEntity);

        // Ortho camera, shadow camera, light
        Light light(ecs, glm::vec3(10.f, 20.f, 0.f));

        // Renderable
        for (int i = 0; i < 30; i++) {
            auto mesh = util::genRandomBool() ? Library::getMesh("cube") : Library::getMesh("sphere");
            Renderable sphere(ecs, mesh.mMesh, glm::vec3(util::genRandom(-10.f, 10.f), util::genRandom(0.5f, 1.f), util::genRandom(-10.f, 10.f)), glm::vec3(0.5f));
            Material material;
            material.mAmbient = glm::vec3(0.3f);
            material.mDiffuse = util::genRandomVec3();
            ecs.addComponent<BoundingBoxComponent>(sphere.mEntity, mesh);
            ecs.addComponent<renderable::PhongShadowRenderable>(sphere.mEntity, *Library::getTexture("black"), material);
            ecs.addComponent<renderable::ShadowCasterRenderable>(sphere.mEntity, *Library::getTexture("black"));
            ecs.addComponent<SelectableComponent>(sphere.mEntity);
        }

        /* Ground plane */
        Renderable receiver(ecs, Library::getMesh("quad").mMesh, glm::vec3(0.f, 0.f, 0.f), glm::vec3(50.f), glm::vec3(-1.56f, 0, 0));
        Material material;
        material.mAmbient = glm::vec3(0.2f);
        material.mDiffuse = glm::vec3(0.7f);
        ecs.addComponent<BoundingBoxComponent>(receiver.mEntity, Library::getMesh("quad"));
        ecs.addComponent<renderable::PhongShadowRenderable>(receiver.mEntity, *Library::getTexture("black"), material);

        /* Systems - order matters! */
        ecs.addSystem<CameraControllerSystem>(); // Update camera
        ecs.addSystem<FrustumSystem>(); // Calculate original frusta bounds
        ecs.addSystem<FrustaFittingSystem>(); // Fit one frusta into another
        ecs.addSystem<FrustumToLineSystem>(); // Create line mesh
        ecs.addSystem<PerspectiveUpdateSystem>(); // Update mock perspective camera

        /* Init renderer */
        renderer.addPreProcessShader<ShadowCasterShader>(shadowMapSize);
        renderer.addSceneShader<PhongShadowShader>();
        renderer.addSceneShader<WireShader>();
        renderer.addSceneShader<LineShader>().mActive = true;
    }
}
