#include "Engine/Engine.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/AlphaTestShader.hpp"

#include "Renderer/GLObjects/Material.hpp"

#include "glm/gtc/matrix_transform.hpp"

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

        Engine::addImGuiFunc("Light", [&]() {
            auto light = Engine::getSingleComponent<LightComponent>();
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
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

    /* Bunny object */
    GameObject* bunny = &Engine::createGameObject();
    Engine::addComponent<SpatialComponent>(bunny, glm::vec3(0.f, 1.0f, 0.f));
    Engine::addComponent<RotationComponent>(bunny, glm::vec3(0.f, 1.0f, 0.f));
    Engine::addComponent<MeshComponent>(bunny, *Library::loadMesh("bunny.obj", true));
    Engine::addComponent<renderable::PhongRenderable>(bunny, *Library::getTexture("black"), Material(glm::vec3(0.2f), glm::vec3(1.f,0.f,1.f)));
    Engine::addComponent<SelectableComponent>(bunny);
    Engine::addComponent<BoundingBoxComponent>(bunny, *Library::loadMesh("bunny.obj"));

    /* Ground plane */
    GameObject* plane = &Engine::createGameObject();
    Engine::addComponent<SpatialComponent>(plane, glm::vec3(0.f), glm::vec3(15.f), glm::vec3(-Util::PI / 2.f, 0.f, 0.f));
    Engine::addComponent<MeshComponent>(plane, *Library::getMesh("quad"));
    Engine::addComponent<renderable::AlphaTestRenderable>(plane, *Library::loadTexture("grid.png"));

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<RotationSystem>();

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<AlphaTestShader>();

    /* Run */
    Engine::run();
    return 0;
}