#include <NeoEngine.hpp>

#include "SurveillanceCamera.hpp"

#include "Shader/PhongShader.hpp"
#include "Shader/LineShader.hpp"
#include "SurveillanceWriteShader.hpp"
#include "SurveillanceReadShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

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
            ImGui::SliderFloat3("Color", glm::value_ptr(light->mColor), 0.f, 1.f);
            ImGui::SliderFloat3("Attenuation", glm::value_ptr(light->mAttenuation), 0.f, 1.f);
        });
    }
};

struct Renderable {
    GameObject *gameObject;

    Renderable(Mesh *mesh) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, glm::vec3(0.f), glm::vec3(1.f));
        NeoEngine::addComponent<MeshComponent>(gameObject, mesh);
        NeoEngine::addComponent<renderable::PhongRenderable>(gameObject);
        NeoEngine::addComponent<MaterialComponent>(gameObject, 0.2f, glm::vec3(1.f, 0.f, 1.f));

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

    Surveillance(std::string name, glm::vec3 pos, glm::vec3 scale, glm::mat3 orientation) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, scale, orientation);
        camera = &NeoEngine::addComponent<SurveillanceCamera>(gameObject, name, 1.f, 100.f);
        // Line
        LineComponent *uLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(1.f, 0.f, 0.f));
        uLine->addNodes({ glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f) });
        LineComponent *vLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(0.f, 1.f, 0.f));
        vLine->addNodes({ glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f) });
        LineComponent *wLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(0.f, 0.f, 1.f));
        wLine->addNodes({ glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f) });
        // Line renderable
        NeoEngine::addComponent<renderable::LineMeshComponent>(gameObject, uLine);
        NeoEngine::addComponent<renderable::LineMeshComponent>(gameObject, vLine);
        NeoEngine::addComponent<renderable::LineMeshComponent>(gameObject, wLine);

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
        });
    }
};

int main() {
    NeoEngine::init("Framebuffers", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    Renderable(Library::getMesh("cube"));
    Surveillance("CamA", glm::vec3(-3, 0, 0), glm::vec3(1.f, 2.f, 1.f), glm::mat3(glm::rotate(glm::mat4(1.f), 1.4f, glm::vec3(0, 1, 0))));
    Surveillance("CamB", glm::vec3(3, 0, 0), glm::vec3(1.f, 2.f, 1.f), glm::mat3(glm::rotate(glm::mat4(1.f), -1.4f, glm::vec3(0, 1, 0))));

    /* Systems - order matters! */
    NeoEngine::addSystem<CameraControllerSystem>();
    NeoEngine::initSystems();

    /* Init Renderer */
    MasterRenderer::init("shaders/", camera.camera, glm::vec3(0.2f, 0.3f, 0.4f));
    MasterRenderer::addPreProcessShader<SurveillanceWriteShader>();
    MasterRenderer::addSceneShader<LineShader>();
    MasterRenderer::addSceneShader<PhongShader>();
    MasterRenderer::addSceneShader<SurveillanceReadShader>("read.vert", "read.frag");

    /* Attach ImGui panes */
    NeoEngine::addDefaultImGuiFunc();

    /* Run */
    NeoEngine::run();

    return 0;
}