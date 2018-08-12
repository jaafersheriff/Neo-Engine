#include <NeoEngine.hpp>

#include "CustomSystem.hpp"

#include "Shader/DiffuseShader.hpp"
#include "Shader/WireframeShader.hpp"
#include "NormalShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Systems */
RenderSystem * renderSystem;

/* Shaders */
DiffuseShader *diffuseShader;
NormalShader *normalShader;
WireframeShader *wireframeShader;

/* Game object definitions */
struct Camera {
    GameObject *gameObject;
    CameraControllerComponent *cameraController;
    CameraComponent *cameraComp;

    Camera(float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        cameraComp = &NeoEngine::addComponent<CameraComponent>(gameObject, fov, near, far);
        cameraController = &NeoEngine::addComponent<CameraControllerComponent>(gameObject, ls, ms);
    }
};

struct Light {
    GameObject *gameObject;
    LightComponent *light;

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos);
        light = &NeoEngine::addComponent<LightComponent>(gameObject, col, att);

        NeoEngine::addImGuiFunc("Light", [&]() {
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -100.f, 100.f)) {
                gameObject->getSpatial()->setPosition(pos);
            }
            glm::vec3 col = light->getColor();
            if (ImGui::SliderFloat3("Color", glm::value_ptr(col), 0.f, 1.f)) {
                light->setColor(col);
            }
            glm::vec3 att = light->getAttenuation();
            ImGui::SliderFloat3("Attenuation", glm::value_ptr(att), 0.f, 1.f);
            light->setAttenuation(att);
        });
    }
};

struct Orient {
    GameObject *gameObject;
    SpatialComponent *spatial;
    MaterialRenderable *renderable;
    Material material;

    Orient(Mesh *mesh) {
        gameObject = &NeoEngine::createGameObject();
        spatial = &NeoEngine::addComponent<SpatialComponent>(gameObject, glm::vec3(0.f), glm::vec3(1.f));
        renderable = &NeoEngine::addComponent<MaterialRenderable>(gameObject, mesh, &material);
        renderable->addShaderType<DiffuseShader>();
        renderable->addShaderType<WireframeShader>();
        renderable->addShaderType<NormalShader>();

        NeoEngine::addImGuiFunc("Mesh", [&]() {
            ImGui::SliderFloat("Ambient ", &material.ambient, 0.f, 1.f);
            ImGui::SliderFloat3("Diffuse Color", glm::value_ptr(material.diffuse), 0.f, 1.f);
            ImGui::SliderFloat3("Specular Color", glm::value_ptr(material.specular), 0.f, 1.f);
            ImGui::SliderFloat("Shine", &material.shine, 0.f, 100.f);
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -10.f, 10.f)) {
                gameObject->getSpatial()->setPosition(pos);
            }
            float scale = gameObject->getSpatial()->getScale().x;
            if (ImGui::SliderFloat("Scale", &scale, 0.f, 10.f)) {
                gameObject->getSpatial()->setScale(glm::vec3(scale));
            }
            static glm::vec3 rot(0.f);
            if (ImGui::SliderFloat3("Rotation", glm::value_ptr(rot), 0.f, 4.f)) {
                glm::mat4 R;
                R = glm::rotate(glm::mat4(1.f), rot.x, glm::vec3(1, 0, 0));
                R *= glm::rotate(glm::mat4(1.f), rot.y, glm::vec3(0, 1, 0));
                R *= glm::rotate(glm::mat4(1.f), rot.z, glm::vec3(0, 0, 1));
                gameObject->getSpatial()->setOrientation(glm::mat3(R));
            }
        });
    }
};

int main() {
    NeoEngine::init("Normal Rendering", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    Orient(Loader::getMesh("bunny.obj"));

    /* Systems - order matters! */
    NeoEngine::addSystem<CustomSystem>();
    renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/");
    diffuseShader = &renderSystem->addShader<DiffuseShader>();
    wireframeShader = &renderSystem->addShader<WireframeShader>();
    normalShader = &renderSystem->addShader<NormalShader>("normal.vert", "normal.frag", "normal.geom");
    NeoEngine::initSystems();

    /* Attach ImGui panes */
    NeoEngine::addImGuiFunc("Stats", [&]() {
        ImGui::Text("FPS: %d", NeoEngine::FPS);
        ImGui::Text("dt: %0.4f", NeoEngine::timeStep);
        if (ImGui::Button("VSync")) {
            Window::toggleVSync();
        }
    });
    NeoEngine::addImGuiFunc("Renderer", [&]() {
        ImGui::Checkbox("Diffuse", &diffuseShader->active);
        ImGui::Checkbox("Wire", &wireframeShader->active);
        ImGui::Checkbox("Normal", &normalShader->active);
        if (normalShader->active) {
            ImGui::SameLine();
            ImGui::SliderFloat("Magnitude", &normalShader->magnitude, 0.f, 1.f);
        }
    });

    /* Run */
    NeoEngine::run();

    return 0;
}