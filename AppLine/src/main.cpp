#include <NeoEngine.hpp>

#include "CustomSystem.hpp"

#include "Shader/LineShader.hpp"

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

struct Line {
    GameObject *gameObject;
    LineRenderable *line;

    Line() {
        gameObject = &NeoEngine::createGameObject();
        line = &NeoEngine::addComponent<LineRenderable>(*gameObject);
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
                    NeoEngine::addComponent<SpatialComponent>(*gameObject);
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

int main() {
    NeoEngine::init("Diffuse Rendering", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 0.01f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Line line;
    line.line->addNode(glm::vec3(0.f));
    line.line->addNode(glm::vec3(0.f, 10.f, 0.f));
    line.line->addNode(glm::vec3(10.f, 10.f, 0.f));

    /* Systems - order matters! */
    NeoEngine::addSystem<CustomSystem>();
    renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/");
    renderSystem->addShader<LineShader>();
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