#include "Engine/Engine.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/WireframeShader.hpp"

#include "NormalShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Shaders */
NormalShader *normalShader;

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
        auto gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos);
        Engine::addComponent<LightComponent>(gameObject, col, att);
    }
};

struct Orient {
    GameObject *gameObject;

    Orient(Mesh *mesh) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, glm::vec3(0.f), glm::vec3(1.f));
        Engine::addComponent<RotationComponent>(gameObject, glm::vec3(0.f, 0.6f, 0.f));
        Engine::addComponent<MeshComponent>(gameObject, *mesh);
        Engine::addComponent<renderable::PhongRenderable>(gameObject, *Library::getTexture("black"));
        Engine::addComponent<renderable::WireframeRenderable>(gameObject);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Geometry Shaders";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    Orient(Library::loadMesh("bunny.obj"));

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<RotationSystem>();

    /* Init renderer */
    Renderer::init("shaders/", glm::vec3(0.2f, 0.3f, 0.4f));
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<NormalShader>("normal.vert", "normal.frag", "normal.geom");

    /* Run */
    Engine::run();

    return 0;
}
