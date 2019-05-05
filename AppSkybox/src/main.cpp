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
        camera = &Engine::addComponent<CameraComponent>(gameObject, fov, near, far);
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
    GameObject *gameObject;

    Reflection(Mesh *m, glm::vec3 pos, float scale = 1.f) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(scale));
        Engine::addComponent<MeshComponent>(gameObject, m);
        Engine::addComponent<renderable::WireframeRenderable>(gameObject);
        Engine::addComponent<ReflectionComponent>(gameObject);
        Engine::addComponent<RotationComponent>(gameObject, glm::vec3(0.f, 0.3f, 0.f));

        Engine::addImGuiFunc("Reflection", [&]() {
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -10.f, 10.f)) {
                gameObject->getSpatial()->setPosition(pos);
            }
            float scale = gameObject->getSpatial()->getScale().x;
            if (ImGui::SliderFloat("Scale", &scale, 0.f, 10.f)) {
                gameObject->getSpatial()->setScale(glm::vec3(scale));
            }

            if (ImGui::Button("Add reflection")) {
                Engine::addComponent<ReflectionComponent>(gameObject);
            }
            ImGui::SameLine();
            if (ImGui::Button("Add wireframe")) {
                Engine::addComponent<renderable::WireframeRenderable>(gameObject);
            }
            if (ImGui::Button("Remove reflection")) {
                Engine::removeComponent<ReflectionComponent>(*gameObject->getComponentByType<ReflectionComponent>());
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove wireframe")) {
                Engine::removeComponent<renderable::WireframeRenderable>(*gameObject->getComponentByType<renderable::WireframeRenderable>());
            }
        });
    }
};

struct Refraction {
    GameObject *gameObject;
    RefractionComponent *refraction;

    Refraction(Mesh *m, glm::vec3 pos, float scale = 1.f) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(scale));
        Engine::addComponent<MeshComponent>(gameObject, m);
        refraction = &Engine::addComponent<RefractionComponent>(gameObject);
        Engine::addComponent<renderable::WireframeRenderable>(gameObject);
        Engine::addComponent<RotationComponent>(gameObject, glm::vec3(0.f, 0.3f, 0.f));

        Engine::addImGuiFunc("Refraction", [&]() {
            ImGui::SliderFloat("Index", &refraction->ratio, 0.f, 1.f);
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -10.f, 10.f)) {
                gameObject->getSpatial()->setPosition(pos);
            }
            float scale = gameObject->getSpatial()->getScale().x;
            if (ImGui::SliderFloat("Scale", &scale, 0.f, 10.f)) {
                gameObject->getSpatial()->setScale(glm::vec3(scale));
            }

            if (ImGui::Button("Add refraction")) {
                refraction = &Engine::addComponent<RefractionComponent>(gameObject);
            }
            ImGui::SameLine();
            if (ImGui::Button("Add wireframe")) {
                Engine::addComponent<renderable::WireframeRenderable>(gameObject);
            }
            if (ImGui::Button("Remove refraction")) {
                Engine::removeComponent<RefractionComponent>(*gameObject->getComponentByType<RefractionComponent>());
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove wireframe")) {
                Engine::removeComponent<renderable::WireframeRenderable>(*gameObject->getComponentByType<renderable::WireframeRenderable>());
            }
 
        });
    }
};

int main() {
    Engine::init("Skybox", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 0.01f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Skybox(Library::getCubemap("arctic_skybox", {"arctic_ft.tga", "arctic_bk.tga", "arctic_up.tga", "arctic_dn.tga", "arctic_rt.tga", "arctic_lf.tga"}));
    Reflection(Library::getMesh("male.obj", true), glm::vec3(-5.f, 0.f, 0.f), 5.f);
    Refraction(Library::getMesh("male.obj", true), glm::vec3(5.f, 0.f, 0.f), 5.f);

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<RotationSystem>();
    Engine::initSystems();

    /* Init renderer and shaders - order matters! */
    Renderer::init("shaders/", camera.camera);
    Renderer::addSceneShader<ReflectionShader>("model.vert", "reflect.frag");
    Renderer::addSceneShader<RefractionShader>("model.vert", "refract.frag");
    Renderer::addSceneShader<SkyboxShader>("skybox.vert", "skybox.frag");
    Renderer::addSceneShader<WireframeShader>();

    /* Attach ImGui panes */
    Engine::addDefaultImGuiFunc();

    /* Run */
    Engine::run();

    return 0;
}