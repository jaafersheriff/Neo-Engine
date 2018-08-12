#include <NeoEngine.hpp>

#include "CustomSystem.hpp"

#include "Shader/LineShader.hpp"
#include "Shader/DiffuseShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

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



struct Line {
    GameObject *gameObject;
    LineRenderable *line;

    Line() {
        gameObject = &NeoEngine::createGameObject();
        line = &NeoEngine::addComponent<LineRenderable>(gameObject);
        line->addShaderType<LineShader>();

        NeoEngine::addImGuiFunc("Line", [&]() {
            ImGui::ColorEdit3("Color", glm::value_ptr(line->lineColor));
            static glm::vec3 node(0.f);
            ImGui::SliderFloat3("Node", glm::value_ptr(node), -10.f, 10.f);
            ImGui::SameLine();
            if (ImGui::Button("Add")) {
                line->addNode(node);
            }
            if (!gameObject->getSpatial()) {
                if (ImGui::Button("Add Spatial")) {
                    NeoEngine::addComponent<SpatialComponent>(gameObject);
                }
            }
            else {
                if (ImGui::Button("Remove Spatial")) {
                    NeoEngine::removeComponent(*gameObject->getSpatial());
                }
                auto spatial = gameObject->getSpatial();
                auto pos = spatial->getPosition();
                if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -10.f, 10.f)) {
                    spatial->setPosition(pos);
                }
                auto scale = spatial->getScale();
                if (ImGui::SliderFloat3("Scale", glm::value_ptr(scale), 0.f, 10.f)) {
                    spatial->setScale(scale);
                }
                auto u = spatial->getU();
                auto v = spatial->getV();
                auto w = spatial->getW();
                if (ImGui::SliderFloat3("U", glm::value_ptr(u), 0.f, 1.f) ||
                    ImGui::SliderFloat3("V", glm::value_ptr(v), 0.f, 1.f) ||
                    ImGui::SliderFloat3("W", glm::value_ptr(w), 0.f, 1.f)) {
                    spatial->setUVW(u, v, w);
                }
            }
        });
    }
};

struct Orient {
    GameObject *gameObject;
    SpatialComponent *spatial;
    MaterialRenderable *renderable;
    LineRenderable *uLine;
    LineRenderable *vLine;
    LineRenderable *wLine;

    Orient(Mesh *mesh) {
        gameObject = &NeoEngine::createGameObject();
        spatial = &NeoEngine::addComponent<SpatialComponent>(gameObject, glm::vec3(0.f), glm::vec3(1.f));
        renderable = &NeoEngine::addComponent<MaterialRenderable>(gameObject, mesh, new Material);
        renderable->addShaderType<DiffuseShader>();
        uLine = &NeoEngine::addComponent<LineRenderable>(gameObject, glm::vec3(1.f, 0.f, 0.f));
        uLine->addNodes({ glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f) });
        vLine = &NeoEngine::addComponent<LineRenderable>(gameObject, glm::vec3(0.f, 1.f, 0.f));
        vLine->addNodes({ glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f) });
        wLine = &NeoEngine::addComponent<LineRenderable>(gameObject, glm::vec3(0.f, 0.f, 1.f));
        wLine->addNodes({ glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f) });

        NeoEngine::addImGuiFunc("Orient", [&]() {
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
    NeoEngine::init("Diffuse Rendering", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 0.01f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    Orient(Loader::getMesh("cube"));

    /* Systems - order matters! */
    NeoEngine::addSystem<CustomSystem>();
    renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/");
    renderSystem->addShader<LineShader>();
    renderSystem->addShader<DiffuseShader>();
    NeoEngine::initSystems();

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