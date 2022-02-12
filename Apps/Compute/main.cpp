#include "Engine/Engine.hpp"

#include "ParticleMeshComponent.hpp"
#include "ParticlesComputeShader.hpp"
#include "ParticleVisShader.hpp"

#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"

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
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Compute";
    config.APP_RES = "res/";
    config.attachEditor = false;
    ECS& ecs = Engine::init(config);

    /* Game objects */
    Camera camera(ecs, 45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    ecs.addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(ecs, glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

    // Create mesh
    {
        auto& go = ecs.createGameObject();
        ecs.addComponent<ParticleMeshComponent>(&go);
        ecs.addComponent<SpatialComponent>(&go, glm::vec3(0.f, 0.0f, 0.f));
    }

    /* Systems - order matters! */
    ecs.addSystem<CameraControllerSystem>();

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addComputeShader<ParticlesComputeShader>("particles.compute");
    Renderer::addSceneShader<ParticleVisShader>("particles.vert", "particles.frag", "particles.geom");

    Engine::addImGuiFunc("Mesh", [](ECS& ecs_) {
        if (auto mesh = ecs_.getComponentTuple<ParticleMeshComponent, SpatialComponent>()) {
            mesh->get<ParticleMeshComponent>()->imGuiEditor();
            mesh->get<SpatialComponent>()->imGuiEditor();
        }
    });

    /* Run */
    Engine::run();
    return 0;
}