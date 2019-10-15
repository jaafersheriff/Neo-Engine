#include <Engine.hpp>

#include "SnowShader.hpp"
#include "SnowSystem.hpp"
#include "Shader/LineShader.hpp"

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
    GameObject *gameObject;
    LightComponent *light;

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos);
        light = &Engine::addComponent<LightComponent>(gameObject, col, att);

        Engine::addImGuiFunc("Light", [&]() {
            light->imGuiEditor();
        });
    }
};

struct Renderable {
    GameObject *gameObject;

    Renderable(Mesh *mesh) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, glm::vec3(0.f), glm::vec3(1.f));
        Engine::addComponent<MeshComponent>(gameObject, mesh);
        Engine::addComponent<MaterialComponent>(gameObject, 0.2f, glm::vec3(1.f, 0.f, 0.f));

        Engine::addImGuiFunc("Mesh", [&]() {
            gameObject->getComponentByType<SpatialComponent>()->imGuiEditor();
        });
    }
};

struct Snow {
    GameObject *gameObject;
    SnowComponent *snow;

    Snow() {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject);
        snow = &Engine::addComponent<SnowComponent>(gameObject);
        auto line = &Engine::addComponent<LineMeshComponent>(gameObject);
        line->addNodes({ {glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f) }, {glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f) } });
        
        Engine::addImGuiFunc("Snow", [&]() {
            snow->imGuiEditor();
            snow->getGameObject().getComponentByType<SpatialComponent>()->imGuiEditor();
        });
    }
};

int main() {
    Engine::init("Snow", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(glm::vec3(0.f, 65.f, 20.f), glm::vec3(1.f), glm::vec3(0.f));
    Renderable(Library::getMesh("male.obj"));
    Snow();

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<SnowSystem>();

    /* Init renderer */
    Renderer::init("shaders/", camera.camera, glm::vec3(0.2f, 0.3f, 0.4f));
    auto snowShader = &Renderer::addSceneShader<SnowShader>("snow.vert", "snow.frag");
    Renderer::addSceneShader<LineShader>();

    /* Attach ImGui panes */
    Engine::addDefaultImGuiFunc();

    /* Run */
    Engine::run();
    return 0;
}