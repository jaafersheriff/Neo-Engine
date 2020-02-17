#include <Engine.hpp>

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/AlphaTestShader.hpp"

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
    config.APP_NAME = "Base";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

    /* Cube object */
    Renderable cube(Library::getMesh("cube"), glm::vec3(0.f, 0.5f, 0.f));
    Engine::addComponent<renderable::PhongRenderable>(cube.gameObject);
    Material material;
    material.ambient = glm::vec3(0.2f);
    material.diffuse = glm::vec3(1.f, 0.f, 1.f);
    Engine::addComponent<MaterialComponent>(cube.gameObject, material);
    Engine::addComponent<SelectableComponent>(cube.gameObject);
    Engine::addComponent<BoundingBoxComponent>(cube.gameObject, Library::getMesh("cube"));
    auto& parent = Engine::addComponent<ParentComponent>(cube.gameObject);

    {
        Renderable child(Library::getMesh("cube"), glm::vec3(0.f, 1.5f, 1.5f));
        parent.childrenObjects.push_back(child.gameObject);
        Engine::addComponent<renderable::PhongRenderable>(child.gameObject);
        Engine::addComponent<SelectableComponent>(child.gameObject);
        Engine::addComponent<BoundingBoxComponent>(child.gameObject, Library::getMesh("cube"));
        Engine::addComponent<ChildComponent>(child.gameObject, cube.gameObject);
        Engine::addComponent<RotationComponent>(child.gameObject, glm::vec3(0.f, 1.f, 0.f));
    }

    /* Ground plane */
    Renderable plane(Library::getMesh("quad"), glm::vec3(0.f), glm::vec3(15.f), glm::vec3(-Util::PI / 2.f, 0.f, 0.f));
    Engine::addComponent<renderable::AlphaTestRenderable>(plane.gameObject);
    Engine::addComponent<DiffuseMapComponent>(plane.gameObject, *Library::loadTexture("grid.png"));

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