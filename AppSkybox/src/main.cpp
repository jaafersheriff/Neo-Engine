#include <NeoEngine.hpp>

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
        GameObject *gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &NeoEngine::addComponent<CameraComponent>(gameObject, fov, near, far);
        NeoEngine::addComponent<CameraControllerComponent>(gameObject, ls, ms);
    }
};

struct Skybox {
    GameObject *gameObject;

    Skybox(Texture *tex) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SkyboxComponent>(gameObject);
        NeoEngine::addComponent<CubeMapComponent>(gameObject, tex);
    }
};

struct Reflection {
    GameObject *gameObject;

    Reflection(Mesh *m, glm::vec3 pos, float scale = 1.f) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(scale));
        NeoEngine::addComponent<MeshComponent>(gameObject, m);
        NeoEngine::addComponent<renderable::WireframeRenderable>(gameObject);
        NeoEngine::addComponent<ReflectionComponent>(gameObject);
        NeoEngine::addComponent<RotationComponent>(gameObject, glm::vec3(0.f, 0.3f, 0.f));

        NeoEngine::addImGuiFunc("Reflection", [&]() {
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -10.f, 10.f)) {
                gameObject->getSpatial()->setPosition(pos);
            }
            float scale = gameObject->getSpatial()->getScale().x;
            if (ImGui::SliderFloat("Scale", &scale, 0.f, 10.f)) {
                gameObject->getSpatial()->setScale(glm::vec3(scale));
            }

            if (ImGui::Button("Add reflection")) {
                NeoEngine::addComponent<ReflectionComponent>(gameObject);
            }
            ImGui::SameLine();
            if (ImGui::Button("Add wireframe")) {
                NeoEngine::addComponent<renderable::WireframeRenderable>(gameObject);
            }
            if (ImGui::Button("Remove reflection")) {
                NeoEngine::removeComponent<ReflectionComponent>(*gameObject->getComponentByType<ReflectionComponent>());
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove wireframe")) {
                NeoEngine::removeComponent<renderable::WireframeRenderable>(*gameObject->getComponentByType<renderable::WireframeRenderable>());
            }
        });
    }
};

struct Refraction {
    GameObject *gameObject;
    RefractionComponent *refraction;

    Refraction(Mesh *m, glm::vec3 pos, float scale = 1.f) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(scale));
        NeoEngine::addComponent<MeshComponent>(gameObject, m);
        refraction = &NeoEngine::addComponent<RefractionComponent>(gameObject);
        NeoEngine::addComponent<renderable::WireframeRenderable>(gameObject);
        NeoEngine::addComponent<RotationComponent>(gameObject, glm::vec3(0.f, 0.3f, 0.f));

        NeoEngine::addImGuiFunc("Refraction", [&]() {
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
                refraction = &NeoEngine::addComponent<RefractionComponent>(gameObject);
            }
            ImGui::SameLine();
            if (ImGui::Button("Add wireframe")) {
                NeoEngine::addComponent<renderable::WireframeRenderable>(gameObject);
            }
            if (ImGui::Button("Remove refraction")) {
                NeoEngine::removeComponent<RefractionComponent>(*gameObject->getComponentByType<RefractionComponent>());
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove wireframe")) {
                NeoEngine::removeComponent<renderable::WireframeRenderable>(*gameObject->getComponentByType<renderable::WireframeRenderable>());
            }
 
        });
    }
};

int main() {
    NeoEngine::init("Skybox", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 0.01f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Skybox(Library::getCubemap("arctic_skybox", {"arctic_ft.tga", "arctic_bk.tga", "arctic_up.tga", "arctic_dn.tga", "arctic_rt.tga", "arctic_lf.tga"}));
    Reflection(Library::getMesh("male.obj", true), glm::vec3(-5.f, 0.f, 0.f), 5.f);
    Refraction(Library::getMesh("male.obj", true), glm::vec3(5.f, 0.f, 0.f), 5.f);

    /* Systems - order matters! */
    NeoEngine::addSystem<CameraControllerSystem>();
    NeoEngine::addSystem<RotationSystem>();
    NeoEngine::initSystems();

    /* Init renderer and shaders - order matters! */
    MasterRenderer::init("shaders/", camera.camera);
    MasterRenderer::addSceneShader<ReflectionShader>("model.vert", "reflect.frag");
    MasterRenderer::addSceneShader<RefractionShader>("model.vert", "refract.frag");
    MasterRenderer::addSceneShader<SkyboxShader>("skybox.vert", "skybox.frag");
    MasterRenderer::addSceneShader<WireframeShader>();

    /* Attach ImGui panes */
    NeoEngine::addDefaultImGuiFunc();

    /* Run */
    NeoEngine::run();

    return 0;
}