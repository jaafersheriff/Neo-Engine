#include "Engine/Engine.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/WireframeShader.hpp"

#include "NormalShader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/TransformationComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Shaders */
NormalShader *normalShader;

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
        auto gameObject = &ecs.createGameObject();
        ecs.addComponent<SpatialComponent>(gameObject, pos);
        ecs.addComponent<LightComponent>(gameObject, col, att);
    }
};

struct Orient {
    GameObject *gameObject;

    Orient(ECS& ecs, Mesh *mesh) {
        gameObject = &ecs.createGameObject();
        ecs.addComponent<SpatialComponent>(gameObject, glm::vec3(0.f), glm::vec3(1.f));
        ecs.addComponent<RotationComponent>(gameObject, glm::vec3(0.f, 0.6f, 0.f));
        ecs.addComponent<MeshComponent>(gameObject, *mesh);
        ecs.addComponent<renderable::PhongRenderable>(gameObject, *Library::getTexture("black"));
        ecs.addComponent<renderable::WireframeRenderable>(gameObject);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Geometry Shaders";
    config.APP_RES = "res/";
    ECS& ecs = Engine::init(config);

    /* Game objects */
    Camera camera(ecs, 45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    ecs.addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(ecs, glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    Orient(ecs, Library::loadMesh("bunny.obj"));

    /* Systems - order matters! */
    ecs.addSystem<CameraControllerSystem>();
    ecs.addSystem<RotationSystem>();

    /* Init renderer */
    Renderer::init("shaders/", glm::vec3(0.2f, 0.3f, 0.4f));
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<NormalShader>("normal.vert", "normal.frag", "normal.geom");

    /* Run */
    Engine::run();

    return 0;
}
