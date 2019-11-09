#include <Engine.hpp>

#include "ParticleMeshComponent.hpp"
#include "ParticlesComputeShader.hpp"
#include "ParticleVisShader.hpp"

using namespace neo;

/* Game object definitions */
struct Camera {
    CameraComponent *camera;
    Camera(float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        GameObject *gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov, Window::getAspectRatio());
        Engine::addComponent<CameraControllerComponent>(gameObject, ls, ms);
    }
};

struct Light {
    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        auto& gameObject = Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(&gameObject, pos);
        Engine::addComponent<LightComponent>(&gameObject, col, att);
    }
};

struct Renderable {
    GameObject *gameObject;

    Renderable(Mesh *mesh, glm::vec3 position = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f), glm::vec3 rotation = glm::vec3(0.f)) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<MeshComponent>(gameObject, mesh);
        Engine::addComponent<SpatialComponent>(gameObject, position, scale, rotation);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Compute";
    config.APP_RES = "res/";
    config.attachEditor = false;
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

    // Create mesh
    {
        auto& go = Engine::createGameObject();
        Engine::addComponent<ParticleMeshComponent>(&go);
        Engine::addComponent<SpatialComponent>(&go, glm::vec3(0.f, 0.0f, 0.f));
    }

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();

    /* Init renderer */
    Renderer::init("shaders/", camera.camera);
    Renderer::addComputeShader<ParticlesComputeShader>("particles.compute");
    Renderer::addSceneShader<ParticleVisShader>("particles.vert", "particles.frag");

    Engine::addImGuiFunc("Mesh", []() {
        if (auto mesh = Engine::getComponentTuple<ParticleMeshComponent, SpatialComponent>()) {
            mesh->get<ParticleMeshComponent>()->imGuiEditor();
            mesh->get<SpatialComponent>()->imGuiEditor();
        }
    });

    /* Run */
    Engine::run();
    return 0;
}