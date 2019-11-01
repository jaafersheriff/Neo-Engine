#include <Engine.hpp>

#include "MetaballsComponent.hpp"
#include "MetaballsSystem.hpp"
#include "MetaballsShader.hpp"

#include "Shader/SkyboxShader.hpp"

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
    config.APP_NAME = "Metaballs";
    config.APP_RES = "res/";
    config.attachEditor = false;
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    /* Skybox */
    {
        GameObject* gameObject = &Engine::createGameObject();
        Engine::addComponent<renderable::SkyboxComponent>(gameObject);
        Engine::addComponent<CubeMapComponent>(gameObject, Library::getCubemap("arctic_skybox", {"arctic_ft.tga", "arctic_bk.tga", "arctic_up.tga", "arctic_dn.tga", "arctic_rt.tga", "arctic_lf.tga"}));
    }

    /* METBALL */
    {
        auto& go = Engine::createGameObject();
        Engine::addComponent<MetaballsComponent>(&go);
        Engine::addComponent<MeshComponent>(&go, new Mesh(GL_TRIANGLES));
        Engine::addComponent<SpatialComponent>(&go, glm::vec3(0.f, 0.5f, 0.f));
    }

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<MetaballsSystem>();

    /* Init renderer */
    Renderer::init("shaders/", camera.camera);
    Renderer::addSceneShader<MetaballsShader>("metaballs.vert", "metaballs.frag");
    Renderer::addSceneShader<SkyboxShader>();

    /* Run */
    Engine::run();
    return 0;
}