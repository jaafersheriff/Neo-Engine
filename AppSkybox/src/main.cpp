#include <Engine.hpp>

#include "ReflectionComponent.hpp"
#include "RefractionComponent.hpp"

#include "SkyboxComponent.hpp"
#include "SkyboxShader.hpp"
#include "ReflectionShader.hpp"
#include "RefractionShader.hpp"
#include "Shader/WireframeShader.hpp"

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

struct Skybox {
    GameObject *gameObject;

    Skybox(Texture *tex) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SkyboxComponent>(gameObject);
        Engine::addComponent<CubeMapComponent>(gameObject, tex);
    }
};

struct Reflection {
    Reflection(Mesh *m, glm::vec3 pos, float scale = 1.f) {
        auto gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(scale));
        Engine::addComponent<MeshComponent>(gameObject, m);
        Engine::addComponent<renderable::WireframeRenderable>(gameObject);
        Engine::addComponent<ReflectionComponent>(gameObject);
        Engine::addComponent<RotationComponent>(gameObject, glm::vec3(0.f, 0.3f, 0.f));

        Engine::addImGuiFunc("Reflection", [&]() {
            if (auto reflection = Engine::getSingleComponent<ReflectionComponent>()) {
                reflection->getGameObject().getComponentByType<SpatialComponent>()->imGuiEditor();
                if (ImGui::Button("Add wireframe")) {
                    Engine::addComponent<renderable::WireframeRenderable>(&reflection->getGameObject());
                }
                ImGui::SameLine();
                if (ImGui::Button("Remove wireframe")) {
                    Engine::removeComponent<renderable::WireframeRenderable>(*reflection->getGameObject().getComponentByType<renderable::WireframeRenderable>());
                }
            }
        });
    }
};

struct Refraction {
    Refraction(Mesh *m, glm::vec3 pos, float scale = 1.f) {
        auto gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(scale));
        Engine::addComponent<MeshComponent>(gameObject, m);
        Engine::addComponent<RefractionComponent>(gameObject);
        Engine::addComponent<renderable::WireframeRenderable>(gameObject);
        Engine::addComponent<RotationComponent>(gameObject, glm::vec3(0.f, 0.3f, 0.f));

        Engine::addImGuiFunc("Refraction", [&]() {
            if (auto refraction = Engine::getSingleComponent<RefractionComponent>()) {
                refraction->imGuiEditor();
                refraction->getGameObject().getComponentByType<SpatialComponent>()->imGuiEditor();

                if (ImGui::Button("Add wireframe")) {
                    Engine::addComponent<renderable::WireframeRenderable>(&refraction->getGameObject());
                }
                ImGui::SameLine();
                if (ImGui::Button("Remove wireframe")) {
                    Engine::removeComponent<renderable::WireframeRenderable>(*refraction->getGameObject().getComponentByType<renderable::WireframeRenderable>());
                }
            }
        });
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Skybox";
    config.APP_RES = "res/";
	Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 0.01f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Skybox(Library::getCubemap("arctic_skybox", {"arctic_ft.tga", "arctic_bk.tga", "arctic_up.tga", "arctic_dn.tga", "arctic_rt.tga", "arctic_lf.tga"}));
    Reflection(Library::getMesh("male.obj", true), glm::vec3(-5.f, 0.f, 0.f), 5.f);
    Refraction(Library::getMesh("male.obj", true), glm::vec3(5.f, 0.f, 0.f), 5.f);

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<RotationSystem>();

    /* Init renderer and shaders - order matters! */
    Renderer::init("shaders/", camera.camera);
    Renderer::addSceneShader<ReflectionShader>("model.vert", "reflect.frag");
    Renderer::addSceneShader<RefractionShader>("model.vert", "refract.frag");
    Renderer::addSceneShader<SkyboxShader>("skybox.vert", "skybox.frag");
    Renderer::addSceneShader<WireframeShader>();

    /* Attach ImGui panes */

    /* Run */
    Engine::run();

    return 0;
}