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
    SkyboxComponent *skybox;

    Skybox(Texture *tex) {
        gameObject = &NeoEngine::createGameObject();
        skybox = &NeoEngine::addComponent<SkyboxComponent>(gameObject);
        skybox->addShaderType<SkyboxShader>();
        NeoEngine::addComponent<CubeMapComponent>(gameObject, tex);
    }
};

struct Reflection {
    GameObject *gameObject;
    RenderableComponent *renderComp;

    Reflection(Mesh *m, glm::vec3 pos, float scale = 1.f) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(scale));
        renderComp = &NeoEngine::addComponent<RenderableComponent>(gameObject, m);
        renderComp->addShaderType<ReflectionShader>();
        renderComp->addShaderType<WireframeShader>();
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
                renderComp->addShaderType<ReflectionShader>();
            }
            ImGui::SameLine();
            if (ImGui::Button("Add wireframe")) {
                renderComp->addShaderType<WireframeShader>();
            }
            if (ImGui::Button("Remove reflection")) {
                renderComp->removeShaderType<ReflectionShader>();
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove wireframe")) {
                renderComp->removeShaderType<WireframeShader>();
            }
        });
    }
};

struct Refraction {
    GameObject *gameObject;
    RenderableComponent *renderComp;
    RefractionComponent *refraction;

    Refraction(Mesh *m, glm::vec3 pos, float scale = 1.f) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(scale));
        renderComp = &NeoEngine::addComponent<RenderableComponent>(gameObject, m);
        renderComp->addShaderType<RefractionShader>();
        renderComp->addShaderType<WireframeShader>();
        refraction = &NeoEngine::addComponent<RefractionComponent>(gameObject);
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
                renderComp->addShaderType<RefractionShader>();
            }
            ImGui::SameLine();
            if (ImGui::Button("Add wireframe")) {
                renderComp->addShaderType<WireframeShader>();
            }
            if (ImGui::Button("Remove refraction")) {
                renderComp->removeShaderType<RefractionShader>();
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove wireframe")) {
                renderComp->removeShaderType<WireframeShader>();
            }
 
        });
    }
};

int main() {
    NeoEngine::init("Skybox", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 0.01f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Skybox(Loader::getTexture("arctic_skybox", {"arctic_ft.tga", "arctic_bk.tga", "arctic_up.tga", "arctic_dn.tga", "arctic_rt.tga", "arctic_lf.tga"}));
    Reflection(Loader::getMesh("male.obj", true), glm::vec3(-5.f, 0.f, 0.f), 5.f);
    Refraction(Loader::getMesh("male.obj", true), glm::vec3(5.f, 0.f, 0.f), 5.f);

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