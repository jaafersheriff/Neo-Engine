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
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov, Window::getAspectRatio());
    }
};

struct Light {
    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        auto& gameObject = Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(&gameObject, pos);
        Engine::addComponent<LightComponent>(&gameObject, col, att);

        Engine::addImGuiFunc("Light", [&]() {
            Engine::getSingleComponent<LightComponent>()->imGuiEditor();
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

void generateObjects(int amount) {
    for (auto& comp : Engine::getComponents<BoundingBoxComponent>()) {
        Engine::removeGameObject(comp->getGameObject());
    }
    for (auto& comp : Engine::getComponents<renderable::WireframeRenderable>()) {
        Engine::removeGameObject(comp->getGameObject());
    }
 
    for (int i = 0; i < amount; i++) {
        glm::vec3 position(Util::genRandom(-15.f, 15.f), 0.f, Util::genRandom(-15.f, 15.f));
        glm::vec3 size = glm::vec3(Util::genRandom(0.5f, 2.f), Util::genRandom(0.5f, 2.f), Util::genRandom(0.5f, 2.f));
        const auto mesh = Library::getMesh("sphere");

        Renderable renderable(mesh, position, size);
        Engine::addComponent<renderable::PhongRenderable>(renderable.gameObject);
        Engine::addComponent<MaterialComponent>(renderable.gameObject, 0.2f, glm::normalize(position), glm::vec3(1.f));
        auto boundingBox = &Engine::addComponent<BoundingBoxComponent>(renderable.gameObject, mesh);
    }
}

int main() {
    EngineConfig config;
    config.APP_NAME = "VFC";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5));
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());
    Engine::addComponent<CameraControllerComponent>(camera.gameObject, 0.4f, 7.f);
    Engine::addComponent<FrustumComponent>(camera.gameObject);

    Light(glm::vec3(-100.f, 100.f, 100.f), glm::vec3(1.f), glm::vec3(0.f, 0.015f, 0.f));

    // random objects
    generateObjects(10);

    /* Ground plane */
    Renderable plane(Library::getMesh("quad"), glm::vec3(0.f), glm::vec3(30.f), glm::vec3(-Util::PI() / 2.f, 0.f, 0.f));
    Engine::addComponent<renderable::AlphaTestRenderable>(plane.gameObject);
    Engine::addComponent<DiffuseMapComponent>(plane.gameObject, *Library::getTexture("grid.png"));

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<FrustumSystem>();
    Engine::addSystem<FrustumToLineSystem>();

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addSceneShader<AlphaTestShader>();
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<LineShader>();
    Renderer::addPostProcessShader<GammaCorrectShader>();

    /* Attach ImGui panes */
    Engine::addImGuiFunc("VFC", [&]() {
        static int slider = 10;
        ImGui::SliderInt("Num objects", &slider, 1, 10000);
        if (ImGui::Button("Regenerate")) {
            generateObjects(slider);
        }
    });

    /* Run */
    Engine::run();
    return 0;
}
