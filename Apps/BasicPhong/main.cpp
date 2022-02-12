#include "Engine/Engine.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/WireframeShader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/AlphaTestRenderable.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/PhongRenderable.hpp"
#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/TransformationComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"

using namespace neo;

/* Game object definitions */
struct Camera {
    GameObject *gameObject;
    CameraControllerComponent *cameraController;
    CameraComponent *cameraComp;

    Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        gameObject = &ecs.createGameObject();
        ecs.addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f), glm::vec3(3.f));
        cameraComp = &ecs.addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
        cameraController = &ecs.addComponent<CameraControllerComponent>(gameObject, ls, ms);
    }
};

struct Light {
    GameObject *gameObject;
    LightComponent *light;

    Light(ECS& ecs, glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        gameObject = &ecs.createGameObject();
        ecs.addComponent<SpatialComponent>(gameObject, pos);
        light = &ecs.addComponent<LightComponent>(gameObject, col, att);
        ecs.addComponent<MeshComponent>(gameObject, *Library::getMesh("sphere"));
        ecs.addComponent<BoundingBoxComponent>(gameObject, *Library::getMesh("sphere"));
        ecs.addComponent<renderable::WireframeRenderable>(gameObject);
        ecs.addComponent<SelectableComponent>(gameObject);

        Engine::addImGuiFunc("Light", [](ECS& ecs_) {
            auto light = ecs_.getSingleComponent<LightComponent>();
            light->imGuiEditor();
            if (auto spatial = light->getGameObject().getComponentByType<SpatialComponent>()) {
                spatial->imGuiEditor();
            }
        });
    }
};

struct Renderable {
    GameObject *gameObject;

    Renderable(ECS& ecs, Mesh *mesh, Texture *tex, glm::vec3 p, float s = 1.f, glm::mat3 o = glm::mat3(1.f)) {
        gameObject = &ecs.createGameObject();
        ecs.addComponent<SpatialComponent>(gameObject, p, glm::vec3(s), o);
        ecs.addComponent<MeshComponent>(gameObject, *mesh);
        Material material;
        material.mAmbient = glm::vec3(0.1f);
        material.mDiffuse = glm::vec3(0.f);
        material.mSpecular = glm::vec3(1.f);
        material.mShininess = 50.f;
        ecs.addComponent<renderable::PhongRenderable>(gameObject, *tex, material);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Basic Phong";
    config.APP_RES = "res/";
    ECS& ecs = Engine::init(config);

    /* Game objects */
    Camera camera(ecs, 45.f, 0.01f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    ecs.addComponent<MainCameraComponent>(camera.gameObject);

    Light(ecs, glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

    Library::loadMesh("mr_krab.obj", true);
    Library::loadTexture("mr_krab.png");
    std::vector<Renderable *> renderables;
    for (int x = -2; x < 3; x++) {
        for (int z = 0; z < 10; z++) {
            renderables.push_back(
                new Renderable(
                    ecs,
                    Library::getMesh("mr_krab.obj"), 
                    Library::getTexture("mr_krab.png"),
                    glm::vec3(x*2, 0, z*2))
            );
        }
    }

    /* Systems - order matters! */
    ecs.addSystem<CameraControllerSystem>();

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<WireframeShader>();

    /* Attach ImGui panes */

    /* Run */
    Engine::run();

    return 0;
}