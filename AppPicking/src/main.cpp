#include <Engine.hpp>

#include "Shader/PhongShader.hpp"
#include "Shader/AlphaTestShader.hpp"
#include "Shader/GammaCorrectShader.hpp"

#include "MouseRaySystem.hpp"
#include "MouseRayComponent.hpp"

#include "SelectingSystem.hpp"
#include "SelectedComponent.hpp"
#include "SelectableComponent.hpp"
#include "SelectableCameraControllerSystem.hpp"
#include "SelectedSystem.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
struct Camera {
    CameraComponent *camera;
    Camera(float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        GameObject *gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
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
            if (auto spatial = light->getGameObject().getComponentByType<SpatialComponent>()) {
                glm::vec3 pos = spatial->getPosition();
                if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -100.f, 100.f)) {
                    spatial->setPosition(pos);
                }
                ImGui::SliderFloat3("Color", glm::value_ptr(light->mColor), 0.f, 1.f);
                ImGui::SliderFloat3("Attenuation", glm::value_ptr(light->mAttenuation), 0.f, 1.f);
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
        Engine::addComponent<renderable::PhongRenderable>(gameObject);
        Engine::addComponent<MaterialComponent>(gameObject, 0.2f, glm::vec3(1.f, 0.f, 1.f), glm::vec3(1.f), 20.f);
        Engine::addComponent<BoundingBoxComponent>(gameObject, mesh->mBuffers.vertices);
        Engine::addComponent<SelectableComponent>(gameObject);
    }
};

int main() {
    Engine::init("Picking", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);

    // New object for mouse ray
    auto& go = Engine::createGameObject();
    Engine::addComponent<MouseRayComponent>(&go);

    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

    /* Cube object */
    auto mesh = Library::getMesh("sphere");
    for (int i = 0; i < 20; i++) {
        Renderable(mesh, glm::vec3(Util::genRandom(-7.5, 7.5), 0.f, Util::genRandom(-7.5, 7.5)));
        Renderable(mesh, glm::vec3(Util::genRandom(-7.5, 7.5), 0.f, Util::genRandom(-7.5, 7.5)));
        Renderable(mesh, glm::vec3(Util::genRandom(-7.5, 7.5), 0.f, Util::genRandom(-7.5, 7.5)));
    }

    /* Ground plane */
    Renderable plane(Library::getMesh("quad"), glm::vec3(0.f), glm::vec3(15.f), glm::vec3(-Util::PI() / 2.f, 0.f, 0.f));
    Engine::addComponent<renderable::AlphaTestRenderable>(plane.gameObject);
    Engine::addComponent<DiffuseMapComponent>(plane.gameObject, Library::getTexture("grid.png"));

    /* Systems - order matters! */
    Engine::addSystem<MouseRaySystem>();
    Engine::addSystem<SelectingSystem>();
    Engine::addSystem<SelectedSystem>();
    Engine::addSystem<SelectableCameraControllerSystem>();
    Engine::initSystems();

    /* Init renderer */
    Renderer::init("shaders/", camera.camera);
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<AlphaTestShader>();
    Renderer::addPostProcessShader<GammaCorrectShader>();

    /* Attach ImGui panes */
    Engine::addDefaultImGuiFunc();

    /* Run */
    Engine::run();
    return 0;
}