#include <Engine.hpp>

#include "Shader/GammaCorrectShader.hpp"
#include "Shader/PhongShader.hpp"
#include "Shader/AlphaTestShader.hpp"
#include "Shader/LineShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
struct Camera {
    GameObject *gameObject;
    CameraComponent *camera;
    Camera(float fov, float near, float far, glm::vec3 pos) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
    }
};

struct Light {
    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        auto& gameObject = Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(&gameObject, pos);
        Engine::addComponent<LightComponent>(&gameObject, col, att);

        Engine::addImGuiFunc("Light", [&]() {
            auto light = Engine::getSingleComponent<LightComponent>();
            glm::vec3 pos = light->getGameObject().getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -100.f, 100.f)) {
                light->getGameObject().getSpatial()->setPosition(pos);
            }
            ImGui::SliderFloat3("Color", glm::value_ptr(light->mColor), 0.f, 1.f);
            ImGui::SliderFloat3("Attenuation", glm::value_ptr(light->mAttenuation), 0.f, 1.f);
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
    Engine::init("Base", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5));
    Engine::addComponent<CameraControllerComponent>(camera.gameObject, 0.4f, 7.f);
    Light(glm::vec3(-100.f, 100.f, 100.f), glm::vec3(1.f), glm::vec3(0.f, 0.015f, 0.f));

    /* Cube object */
    for (int i = 0; i < 1000; i++) {
        glm::vec3 position(Util::genRandom(-15.f, 15.f), 0.f, Util::genRandom(-15.f, 15.f));
        Renderable cube(Library::getMesh("cube"), position, glm::vec3(Util::genRandom(0.2f, 1.f)));
        Engine::addComponent<renderable::PhongRenderable>(cube.gameObject);
        Engine::addComponent<MaterialComponent>(cube.gameObject, 0.2f, glm::normalize(position), glm::vec3(1.f));
    }

    /* Ground plane */
    Renderable plane(Library::getMesh("quad"), glm::vec3(0.f), glm::vec3(30.f), glm::vec3(-Util::PI() / 2.f, 0.f, 0.f));
    Engine::addComponent<renderable::AlphaTestRenderable>(plane.gameObject);
    Engine::addComponent<DiffuseMapComponent>(plane.gameObject, Library::getTexture("grid.png"));

    Camera mockCamera(45.f, 3.f, 30.f, glm::vec3(0.f, 2.6f, 15.f));
    Engine::addComponent<FrustumBoundsComponent>(mockCamera.gameObject);
    auto lineComp = &Engine::addComponent<LineComponent>(mockCamera.gameObject);
    Engine::addComponent<renderable::LineMeshComponent>(mockCamera.gameObject, lineComp, glm::vec3(0.f, 1.f, 1.f));

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<FrustumBoundsSystem>();
    Engine::addSystem<FrustumBoundsToLineSystem>();
    Engine::initSystems();

    /* Init renderer */
    Renderer::init("shaders/", camera.camera);
    Renderer::addSceneShader<AlphaTestShader>();
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<LineShader>();
    Renderer::addPostProcessShader<GammaCorrectShader>();

    /* Attach ImGui panes */
    Engine::addDefaultImGuiFunc();

    /* Run */
    Engine::run();
    return 0;
}