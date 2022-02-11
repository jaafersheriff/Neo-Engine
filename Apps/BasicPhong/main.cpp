#include "Engine/Engine.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/WireframeShader.hpp"

using namespace neo;

/* Game object definitions */
struct Camera {
    GameObject *gameObject;
    CameraControllerComponent *cameraController;
    CameraComponent *cameraComp;

    Camera(float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f), glm::vec3(3.f));
        cameraComp = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov, Window::getAspectRatio());
        cameraController = &Engine::addComponent<CameraControllerComponent>(gameObject, ls, ms);
    }
};

struct Light {
    GameObject *gameObject;
    LightComponent *light;

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos);
        light = &Engine::addComponent<LightComponent>(gameObject, col, att);
        Engine::addComponent<MeshComponent>(gameObject, *Library::getMesh("sphere"));
        Engine::addComponent<BoundingBoxComponent>(gameObject, *Library::getMesh("sphere"));
        Engine::addComponent<renderable::WireframeRenderable>(gameObject);
        Engine::addComponent<SelectableComponent>(gameObject);

        Engine::addImGuiFunc("Light", [&]() {
            light->imGuiEditor();
        });
    }
};

struct Renderable {
    GameObject *gameObject;

    Renderable(Mesh *mesh, Texture *tex, glm::vec3 p, float s = 1.f, glm::mat3 o = glm::mat3(1.f)) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, p, glm::vec3(s), o);
        Engine::addComponent<MeshComponent>(gameObject, *mesh);
        Material material;
        material.mAmbient = glm::vec3(0.1f);
        material.mDiffuse = glm::vec3(0.f);
        material.mSpecular = glm::vec3(1.f);
        material.mShininess = 50.f;
        Engine::addComponent<renderable::PhongRenderable>(gameObject, *tex, material);
        Engine::addComponent<SelectableComponent>(gameObject);
        Engine::addComponent<BoundingBoxComponent>(gameObject, *mesh);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Basic Phong";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 0.01f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(camera.gameObject);

    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));


    Library::loadMesh("mr_krab.obj", true);
    Library::loadTexture("mr_krab.png");
    std::vector<Renderable *> renderables;
    for (int x = -2; x < 3; x++) {
        for (int z = 0; z < 10; z++) {
            renderables.push_back(
                new Renderable(
                    Library::getMesh("mr_krab.obj"), 
                    Library::getTexture("mr_krab.png"),
                    glm::vec3(x*2, 0, z*2))
            );
        }
    }

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<WireframeShader>();

    /* Attach ImGui panes */

    /* Run */
    Engine::run();

    return 0;
}