#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
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
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/AlphaTestShader.hpp"

#include "Renderer/GLObjects/Material.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
struct Camera {
    CameraComponent *camera;
    Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        GameObject *gameObject = &ecs.createGameObject();
        ecs.addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &ecs.addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
        ecs.addComponent<CameraControllerComponent>(gameObject, ls, ms);
    }
};

struct Light {
    Light(ECS& ecs, glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        auto& gameObject = ecs.createGameObject();
        ecs.addComponent<SpatialComponent>(&gameObject, pos);
        ecs.addComponent<LightComponent>(&gameObject, col, att);

        Engine::addImGuiFunc("Light", [&]() {
            auto light = ecs.getSingleComponent<LightComponent>();
            light->imGuiEditor();
            if (auto spatial = light->getGameObject().getComponentByType<SpatialComponent>()) {
                spatial->imGuiEditor();
            }
        });
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Base";
    config.APP_RES = "res/";
    auto& ecs = Engine::init(config);

    /* Game objects */
    Camera camera(ecs, 45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    ecs.addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(ecs, glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

    /* Bunny object */
    GameObject* bunny = &ecs.createGameObject();
    ecs.addComponent<SpatialComponent>(bunny, glm::vec3(0.f, 1.0f, 0.f));
    ecs.addComponent<RotationComponent>(bunny, glm::vec3(0.f, 1.0f, 0.f));
    ecs.addComponent<MeshComponent>(bunny, *Library::loadMesh("bunny.obj", true));
    ecs.addComponent<renderable::PhongRenderable>(bunny, *Library::getTexture("black"), Material(glm::vec3(0.2f), glm::vec3(1.f,0.f,1.f)));
    ecs.addComponent<SelectableComponent>(bunny);
    ecs.addComponent<BoundingBoxComponent>(bunny, *Library::loadMesh("bunny.obj"));

    /* Ground plane */
    GameObject* plane = &ecs.createGameObject();
    ecs.addComponent<SpatialComponent>(plane, glm::vec3(0.f), glm::vec3(15.f), glm::vec3(-util::PI / 2.f, 0.f, 0.f));
    ecs.addComponent<MeshComponent>(plane, *Library::getMesh("quad"));
    ecs.addComponent<renderable::AlphaTestRenderable>(plane, *Library::loadTexture("grid.png"));

    /* Systems - order matters! */
    ecs.addSystem<CameraControllerSystem>();
    ecs.addSystem<RotationSystem>();

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<AlphaTestShader>();

    /* Run */
    Engine::run();
    return 0;
}