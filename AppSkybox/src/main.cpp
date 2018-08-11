#include <NeoEngine.hpp>

#include "CustomSystem.hpp"
#include "ReflectionRenderable.hpp"
#include "RefractionRenderable.hpp"

#include "SkyboxComponent.hpp"
#include "SkyboxShader.hpp"
#include "ReflectionShader.hpp"
#include "RefractionShader.hpp"

#include "Shader/WireframeShader.hpp"

using namespace neo;

/* Systems */
RenderSystem * renderSystem;

/* Game object definitions */
struct Camera {
    GameObject *gameObject;
    CameraControllerComponent *cameraController;
    CameraComponent *cameraComp;

    Camera(float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(*gameObject, pos, glm::vec3(1.f));
        cameraComp = &NeoEngine::addComponent<CameraComponent>(*gameObject, fov, near, far);
        cameraController = &NeoEngine::addComponent<CameraControllerComponent>(*gameObject, ls, ms);
    }
};

struct Skybox {
    GameObject *gameObject;
    SkyboxComponent *skybox;

    Skybox(Texture *tex) {
        gameObject = &NeoEngine::createGameObject();
        skybox = &NeoEngine::addComponent<SkyboxComponent>(*gameObject, tex);
        skybox->addShaderType<SkyboxShader>();
    }
};

struct Reflection {
    GameObject *gameObject;
    ReflectionRenderable *renderComp;

    Reflection(Mesh *m, glm::vec3 pos, float scale = 1.f) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(*gameObject, pos, glm::vec3(scale));
        renderComp = &NeoEngine::addComponentAs<ReflectionRenderable, RenderableComponent>(*gameObject, m);
        renderComp->addShaderType<ReflectionShader>();
        renderComp->addShaderType<WireframeShader>();

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
    RefractionRenderable *renderComp;

    Refraction(Mesh *m, glm::vec3 pos, float scale = 1.f) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(*gameObject, pos, glm::vec3(scale));
        renderComp = &NeoEngine::addComponentAs<RefractionRenderable, RenderableComponent>(*gameObject, m);
        renderComp->addShaderType<RefractionShader>();
        renderComp->addShaderType<WireframeShader>();

        NeoEngine::addImGuiFunc("Refraction", [&]() {
            ImGui::SliderFloat("Index", &renderComp->ratio, 0.f, 1.f);
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
    Skybox(Loader::getTexture("arctic_skybox", {"arctic_ft.tga", "arctic_bk.tga", "arctic_up.tga", "arctic_dn.tga", "arctic_rt.tga", "arctic_lf.tga"}, GL_CLAMP_TO_EDGE));
    Reflection(Loader::getMesh("male.obj", true), glm::vec3(-5.f, 0.f, 0.f), 5.f);
    Refraction(Loader::getMesh("male.obj", true), glm::vec3(5.f, 0.f, 0.f), 5.f);

    /* Systems - order matters! */
    NeoEngine::addSystem<CustomSystem>();
    renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/");
    NeoEngine::initSystems();

    /* Shaders - order matters! */
    renderSystem->addShader<ReflectionShader>("model.vert", "reflect.frag");
    renderSystem->addShader<RefractionShader>("model.vert", "refract.frag");
    renderSystem->addShader<SkyboxShader>("skybox.vert", "skybox.frag");
    renderSystem->addShader<WireframeShader>();

    /* Attach ImGui panes */
    NeoEngine::addImGuiFunc("Stats", [&]() {
        ImGui::Text("FPS: %d", NeoEngine::FPS);
        ImGui::Text("dt: %0.4f", NeoEngine::timeStep);
        if (ImGui::Button("VSync")) {
            Window::toggleVSync();
        }
    });
    NeoEngine::addImGuiFunc("Render System", [&]() {
        ImGui::Text("Shaders:  %d", renderSystem->shaders.size());
        for (auto it(renderSystem->shaders.begin()); it != renderSystem->shaders.end(); ++it) {
            ImGui::Checkbox(it->get()->name.c_str(), &it->get()->active);
        }
    });
    /* Run */
    NeoEngine::run();

    return 0;
}