#include <NeoEngine.hpp>

#include "CustomSystem.hpp"

#include "SnowShader.hpp"
#include "Shader/LineShader.hpp"

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
    Material material = Material(0.2f, glm::vec3(1.f, 0.f, 0.f));

    Renderable(Mesh *mesh) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, glm::vec3(0.f), glm::vec3(1.f));
        renderable = &NeoEngine::addComponent<RenderableComponent>(gameObject, mesh);
        renderable->addShaderType<SnowShader>();
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

struct Snow {
    GameObject *gameObject;
    SnowComponent *snow;

    Snow() {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject);
        snow = &NeoEngine::addComponent<SnowComponent>(gameObject);
        LineComponent *vLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(0.f, 1.f, 0.f));
        vLine->addNodes({ glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f) });
        NeoEngine::addComponent<LineRenderable>(gameObject, vLine);
        
        NeoEngine::addImGuiFunc("Snow", [&]() {
            ImGui::SliderFloat("Snow size", &snow->snowSize, 1.f, 0.f);
            ImGui::SliderFloat3("Snow color", glm::value_ptr(snow->snowColor), 0.f, 1.f);
            ImGui::SliderFloat("Height", &snow->height, 0.f, .25f);
            ImGui::SliderFloat3("Rim color", glm::value_ptr(snow->rimColor), 0.f, 1.f);
            ImGui::SliderFloat("Rim power", &snow->rimPower, 0.f, 25.f);
            static glm::vec3 rot(0.f);
            if (ImGui::SliderFloat3("Rotation", glm::value_ptr(rot), -Util::PI(), Util::PI())) {
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
    NeoEngine::init("Snow", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Light(glm::vec3(0.f, 15.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    Renderable(Loader::getMesh("male.obj"));
    Snow();

    /* Systems - order matters! */
    NeoEngine::addSystem<CustomSystem>();
    renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/", camera.camera);
    auto snowShader = &renderSystem->addSceneShader<SnowShader>("snow.vert", "snow.frag");
    renderSystem->addSceneShader<LineShader>();
    NeoEngine::initSystems();

    /* Attach ImGui panes */
    NeoEngine::addImGuiFunc("Stats", [&]() {
        ImGui::Text("FPS: %d", Util::FPS);
        ImGui::Text("dt: %0.4f", Util::timeStep);
        if (ImGui::Button("VSync")) {
            Window::toggleVSync();
        }
    });
    NeoEngine::addImGuiFunc("Render System", [&]() {
        ImGui::Text("Shaders:  %d", renderSystem->preShaders.size() + renderSystem->sceneShaders.size());
        for (auto it(renderSystem->preShaders.begin()); it != renderSystem->preShaders.end(); ++it) {
            ImGui::Checkbox(it->get()->name.c_str(), &it->get()->active);
        }
        for (auto it(renderSystem->sceneShaders.begin()); it != renderSystem->sceneShaders.end(); ++it) {
            ImGui::Checkbox(it->get()->name.c_str(), &it->get()->active);
        }
    });

    /* Run */
    NeoEngine::run();
    return 0;
}