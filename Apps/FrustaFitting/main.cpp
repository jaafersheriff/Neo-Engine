#include "Engine/Engine.hpp"

#include "PerspectiveUpdateSystem.hpp"

#include "Renderer/Shader/ShadowCasterShader.hpp"
#include "Renderer/Shader/PhongShadowShader.hpp"
#include "Renderer/Shader/LineShader.hpp"
#include "Renderer/Shader/WireFrameShader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
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
struct Camera {
    GameObject *gameObject;
    CameraComponent *camera;

    Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos) {
        gameObject = &ecs.createGameObject();
        ecs.addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &ecs.addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
    }
};

struct Light {
    Light(ECS& ecs, glm::vec3 position) {
        // Light object
        auto lightObject = &ecs.createGameObject();
        auto& spatial = ecs.addComponent<SpatialComponent>(lightObject, position, glm::vec3(1.f));
        spatial.setLookDir(glm::vec3(0.f, -0.5f, 0.7f));
        ecs.addComponent<LightComponent>(lightObject, glm::vec3(1.f), glm::vec3(0.4f, 0.2f, 0.f));

        // Shadow camera object
        auto cameraObject = &ecs.createGameObject();
        ecs.addComponentAs<OrthoCameraComponent, CameraComponent>(cameraObject, -2.f, 2.f, -4.f, 2.f, 0.1f, 5.f);
        ecs.addComponent<SpatialComponent>(cameraObject, position, glm::vec3(1.f));
        ecs.addComponent<FrustumComponent>(cameraObject);
        ecs.addComponent<FrustumFitReceiverComponent>(cameraObject);
        ecs.addComponent<LineMeshComponent>(cameraObject, glm::vec3(1.f, 0.f, 1.f));
        ecs.addComponent<ShadowCameraComponent>(cameraObject);

        Engine::addImGuiFunc("Light", [](ECS& ecs_) {
            auto light = ecs_.getSingleComponent<LightComponent>();
            auto spatial = light->getGameObject().getComponentByType<SpatialComponent>();
            light->imGuiEditor();
            glm::vec3 lookdir = spatial->getLookDir();
            ImGui::SliderFloat3("lookdir", &lookdir[0], -1.f, 1.f);
            spatial->setLookDir(lookdir);
        });
    }
};

struct Renderable {
    GameObject *gameObject;

    Renderable(ECS& ecs, Mesh *mesh, glm::vec3 position = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f), glm::vec3 rotation = glm::vec3(0.f)) {
        gameObject = &ecs.createGameObject();
        ecs.addComponent<MeshComponent>(gameObject, *mesh);
        ecs.addComponent<SpatialComponent>(gameObject, position, scale, rotation);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "ShadowFrustaFitting";
    config.APP_RES = "res/";
    ECS& ecs = Engine::init(config);

    /* Game objects */
    Camera sceneCamera(ecs, 45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5));
    ecs.addComponent<CameraControllerComponent>(sceneCamera.gameObject, 0.4f, 7.f);
    ecs.addComponent<MainCameraComponent>(sceneCamera.gameObject);
    
    // Perspective camera
    Camera mockCamera(ecs, 50.f, 0.1f, 5.f, glm::vec3(0.f, 2.f, -0.f));
    &ecs.addComponent<LineMeshComponent>(mockCamera.gameObject, glm::vec3(0.f, 1.f, 1.f));
    ecs.addComponent<FrustumComponent>(mockCamera.gameObject);
    ecs.addComponent<FrustumFitSourceComponent>(mockCamera.gameObject);

    // Ortho camera, shadow camera, light
    Light light(ecs, glm::vec3(10.f, 20.f, 0.f));

    // Renderable
    for (int i = 0; i < 30; i++) {
        Renderable sphere(ecs, util::genRandomBool() ? Library::getMesh("cube") : Library::getMesh("sphere"), glm::vec3(util::genRandom(-10.f, 10.f), util::genRandom(0.5f, 1.f), util::genRandom(-10.f, 10.f)), glm::vec3(0.5f));
        Material material;
        material.mAmbient = glm::vec3(0.3f);
        material.mDiffuse = util::genRandomVec3();
        ecs.addComponent<renderable::PhongShadowRenderable>(sphere.gameObject, *Library::getTexture("black"), material);
        ecs.addComponent<renderable::ShadowCasterRenderable>(sphere.gameObject, *Library::getTexture("black"));
    }

    /* Ground plane */
    Renderable receiver(ecs, Library::getMesh("quad"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(50.f), glm::vec3(-1.56f, 0, 0));
    Material material;
    material.mAmbient = glm::vec3(0.2f);
    material.mDiffuse = glm::vec3(0.7f);
    ecs.addComponent<renderable::PhongShadowRenderable>(receiver.gameObject, *Library::getTexture("black"), material);

    /* Systems - order matters! */
    ecs.addSystem<CameraControllerSystem>(); // Update camera
    ecs.addSystem<FrustumSystem>(); // Calculate original frusta bounds
    ecs.addSystem<FrustaFittingSystem>(); // Fit one frusta into another
    ecs.addSystem<FrustumToLineSystem>(); // Create line mesh
    auto& perspectiveUpdate = ecs.addSystem<PerspectiveUpdateSystem>(); // Update mock perspective camera

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addPreProcessShader<ShadowCasterShader>(shadowMapSize);
    Renderer::addSceneShader<PhongShadowShader>();
    Renderer::addSceneShader<LineShader>().mActive = true;

    /* Attach ImGui panes */
    Engine::addImGuiFunc("SceneCamera", [&](ECS& ecs_) {
        if (ImGui::Button("Set scene")) {
            ecs_.removeComponent<MainCameraComponent>(*mockCamera.gameObject->getComponentByType<MainCameraComponent>());
            ecs_.removeComponent<CameraControllerComponent>(*mockCamera.gameObject->getComponentByType<CameraControllerComponent>());
            ecs_.removeComponent<FrustumFitSourceComponent>(*mockCamera.gameObject->getComponentByType<FrustumFitSourceComponent>());
            if (!sceneCamera.gameObject->getComponentByType<MainCameraComponent>()) {
                ecs_.addComponent<MainCameraComponent>(sceneCamera.gameObject);
            }
            if (!sceneCamera.gameObject->getComponentByType<CameraControllerComponent>()) {
                ecs_.addComponent<CameraControllerComponent>(sceneCamera.gameObject, 0.4f, 7.f);
            }
            if (!sceneCamera.gameObject->getComponentByType<FrustumFitSourceComponent>()) {
                ecs_.addComponent<FrustumFitSourceComponent>(sceneCamera.gameObject);
            }
        }
        if (ImGui::Button("Set perspective")) {
            ecs_.removeComponent<MainCameraComponent>(*sceneCamera.gameObject->getComponentByType<MainCameraComponent>());
            ecs_.removeComponent<CameraControllerComponent>(*sceneCamera.gameObject->getComponentByType<CameraControllerComponent>());
            ecs_.removeComponent<FrustumFitSourceComponent>(*sceneCamera.gameObject->getComponentByType<FrustumFitSourceComponent>());
            if (!mockCamera.gameObject->getComponentByType<MainCameraComponent>()) {
                ecs_.addComponent<MainCameraComponent>(mockCamera.gameObject);
            }
            if (!mockCamera.gameObject->getComponentByType<CameraControllerComponent>()) {
                ecs_.addComponent<CameraControllerComponent>(mockCamera.gameObject, 0.4f, 7.f);
            }
            if (!mockCamera.gameObject->getComponentByType<FrustumFitSourceComponent>()) {
                ecs_.addComponent<FrustumFitSourceComponent>(mockCamera.gameObject);
            }
        }

    });

    Engine::addImGuiFunc("PerspectiveCamera", [&](ECS& ecs_) {
        NEO_UNUSED(ecs_);
        auto spatial = mockCamera.gameObject->getComponentByType<SpatialComponent>();
        auto camera = dynamic_cast<PerspectiveCameraComponent*>(mockCamera.camera);
        ImGui::Checkbox("Auto update", &perspectiveUpdate.mUpdatePerspective);
        spatial->imGuiEditor();
        camera->imGuiEditor();
    });

    /* Run */
    Engine::run();
    return 0;
}
