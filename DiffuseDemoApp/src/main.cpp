#include <NeoEngine.hpp>
#include "GameObject/GameObject.hpp"

#include "DiffuseRenderable.hpp"
#include "DiffuseShader.hpp"
#include "CameraSystem.hpp"

#include <glm/gtc/matrix_transform.hpp>

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

struct Light {
    GameObject *gameObject;
    LightComponent *light;

    Light(glm::vec3 pos) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(*gameObject, pos);
        light = &NeoEngine::addComponent<LightComponent>(*gameObject);

        NeoEngine::addImGuiFunc("Light", [&]() {
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -10.f, 10.f)) {
                gameObject->getSpatial()->setPosition(pos);
            }
            glm::vec3 col = light->getColor();
            if (ImGui::SliderFloat3("Color", glm::value_ptr(col), 0.f, 1.f)) {
                light->setColor(col);
            }
            glm::vec3 att = light->getAttenuation();
            if (ImGui::SliderFloat3("Attenuation", glm::value_ptr(att), 0.f, 100.f)) {
                light->setAttenuation(att);
            }
        });
    }
};

struct Renderable {
    GameObject *gameObject;
    DiffuseRenderable *renderComp;

    Renderable(Mesh *m, Texture *t, glm::vec3 p, float s, glm::mat3 o = glm::mat3()) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(*gameObject, p, glm::vec3(s), o);
        renderComp = &NeoEngine::addComponent<DiffuseRenderable>(*gameObject, m, t);
        renderComp->addShaderType<DiffuseShader>();
    }

    void attachImGui(const std::string & name) {
        NeoEngine::addImGuiFunc(name, [&]() {
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -10.f, 10.f)) {
                gameObject->getSpatial()->setPosition(pos);
            }
            float scale = gameObject->getSpatial()->getScale().x;
            if (ImGui::SliderFloat("Scale", &scale, 0.f, 10.f)) {
                gameObject->getSpatial()->setScale(glm::vec3(scale));
            }
            auto material = renderComp->getModelTexture().getMaterial();
            ImGui::SliderFloat("Ambient ", &material->ambient, 0.f, 1.f);
            ImGui::SliderFloat3("Diffuse Color", glm::value_ptr(material->diffuse), 0.f, 1.f);
            ImGui::SliderFloat3("Specular Color", glm::value_ptr(material->specular), 0.f, 1.f);
            ImGui::SliderFloat("Shine", &material->shine, 0.f, 100.f);
        });
    }
};

int main() {
    NeoEngine::init("Diffuse Rendering", "res/", 1280, 720);

    /* Init engine-necessary components */
    Camera camera(45.f, 0.01f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
   
    /* Systems - order matters! */
    NeoEngine::addSystem<CameraSystem>();
    renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/");
    renderSystem->addShader<DiffuseShader>("diffuse.vert", "diffuse.frag");
    NeoEngine::initSystems();

    /* Game objects */
    Renderable krab = Renderable(Loader::getMesh("mr_krab.obj", true), Loader::getTexture("mr_krab.png"), glm::vec3(0.f), 1.f);
    krab.attachImGui("Krab");
    Light(glm::vec3(0.f));

    /* Attach ImGui panes */
    NeoEngine::addImGuiFunc("Stats", [&]() {
        ImGui::Text("FPS: %d", NeoEngine::FPS);
        ImGui::Text("dt: %0.4f", NeoEngine::timeStep);
        if (ImGui::Button("VSync")) {
            Window::toggleVSync();
        }
    });

    /* Run */
    NeoEngine::run();

    return 0;
}