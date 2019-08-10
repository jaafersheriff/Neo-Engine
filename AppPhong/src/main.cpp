#include <Engine.hpp>

#include "Shader/PhongShader.hpp"
#include "Shader/WireframeShader.hpp"

using namespace neo;

/* Game object definitions */
struct Camera {
    GameObject *gameObject;
    CameraControllerComponent *cameraController;
    CameraComponent *cameraComp;

    Camera(float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        cameraComp = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
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
        Engine::addComponent<MeshComponent>(gameObject, Library::getMesh("cube"));

        Engine::addImGuiFunc("Light", [&]() {
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -100.f, 100.f)) {
                gameObject->getSpatial()->setPosition(pos);
            }
            ImGui::SliderFloat3("Color", glm::value_ptr(light->mColor), 0.f, 1.f);
            ImGui::SliderFloat3("Attenuation", glm::value_ptr(light->mAttenuation), 0.f, 1.f);
        });
    }
};

struct Renderable {
    GameObject *gameObject;

    Renderable(Mesh *mesh, Texture *tex, glm::vec3 p, float s = 1.f, glm::mat3 o = glm::mat3()) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, p, glm::vec3(s), o);
        Engine::addComponent<MeshComponent>(gameObject, mesh);
        Engine::addComponent<renderable::PhongRenderable>(gameObject);
        Engine::addComponent<MaterialComponent>(gameObject);
        Engine::addComponent<DiffuseMapComponent>(gameObject, tex);
    }
};

int main() {
    Engine::init("Phong Rendering", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 0.01f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

    std::vector<Renderable *> renderables;
    for (int x = -2; x < 3; x++) {
        for (int z = 0; z < 10; z++) {
            renderables.push_back(
                new Renderable(
                    Library::getMesh("mr_krab.obj", true), 
                    Library::getTexture("mr_krab.png"),
                    glm::vec3(x*2, 0, z*2))
            );
        }
    }

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::initSystems();

    /* Init renderer */
    Renderer::init("shaders/", camera.cameraComp);
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<WireframeShader>();

    /* Attach ImGui panes */
    Engine::addDefaultImGuiFunc();

    /* Run */
    Engine::run();

    return 0;
}