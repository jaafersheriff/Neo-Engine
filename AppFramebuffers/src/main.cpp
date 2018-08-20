#include <NeoEngine.hpp>

#include "SurveillanceCamera.hpp"

#include "CustomSystem.hpp"

#include "Shader/DiffuseShader.hpp"
#include "Shader/LineShader.hpp"
#include "SurveillanceWriteShader.hpp"
#include "SurveillanceReadShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Systems */
RenderSystem * renderSystem;

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

struct Renderable {
    GameObject *gameObject;
    RenderableComponent *renderable;
    Material material = Material(0.2f, glm::vec3(1,0,1));

    Renderable(Mesh *mesh) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, glm::vec3(0.f), glm::vec3(1.f));
        renderable = &NeoEngine::addComponent<RenderableComponent>(gameObject, mesh);
        renderable->addShaderType<DiffuseShader>();
        NeoEngine::addComponent<MaterialComponent>(gameObject, &material);

        NeoEngine::addImGuiFunc("Mesh", [&]() {
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

struct Surveillance {
    GameObject *gameObject;
    SurveillanceCamera *camera;

    Surveillance(glm::vec3 pos, glm::vec3 scale, glm::mat3 orientation) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, scale, orientation);
        camera = &NeoEngine::addComponent<SurveillanceCamera>(gameObject, 1.f, 100.f);
        // Line
        LineComponent *uLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(1.f, 0.f, 0.f));
        uLine->addNodes({ glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f) });
        LineComponent *vLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(0.f, 1.f, 0.f));
        vLine->addNodes({ glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f) });
        LineComponent *wLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(0.f, 0.f, 1.f));
        wLine->addNodes({ glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f) });
        // Line renderable
        NeoEngine::addComponent<LineRenderable>(gameObject, uLine);
        NeoEngine::addComponent<LineRenderable>(gameObject, vLine);
        NeoEngine::addComponent<LineRenderable>(gameObject, wLine);
    }

    void addImGui(std::string name) {
        NeoEngine::addImGuiFunc(name, [&]() {
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
            float imscale = 0.12f;
            ImGui::Image((ImTextureID)camera->colorBuffer->textureId, ImVec2(imscale * camera->colorBuffer->width, imscale * camera->colorBuffer->height));
            ImGui::SameLine();
            ImGui::Image((ImTextureID)camera->depthBuffer->textureId, ImVec2(imscale * camera->depthBuffer->width, imscale * camera->depthBuffer->height));
        });
    }
};

int main() {
    NeoEngine::init("Framebuffers", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    Renderable(Loader::getMesh("cube"));
    Surveillance a(glm::vec3(-3, 0, 0), glm::vec3(1.f, 2.f, 1.f), glm::mat3(glm::rotate(glm::mat4(1.f), 1.4f, glm::vec3(0, 1, 0))));
    a.addImGui("CamA");
    Surveillance b(glm::vec3(3, 0, 0), glm::vec3(1.f, 2.f, 1.f), glm::mat3(glm::rotate(glm::mat4(1.f), -1.4f, glm::vec3(0, 1, 0))));
    b.addImGui("CamB");

    /* Systems - order matters! */
    NeoEngine::addSystem<CustomSystem>();
    renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/", camera.camera);
    NeoEngine::initSystems();

    /* Add shaders */
    renderSystem->addShader<SurveillanceWriteShader, ShaderTypes::PREPROCESS>();
    renderSystem->addShader<LineShader>();
    renderSystem->addShader<DiffuseShader>();
    renderSystem->addShader<SurveillanceReadShader>("read.vert", "read.frag");

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