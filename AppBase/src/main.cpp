#include <Engine.hpp>

#include "Shader/PhongShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

/* Game object definitions */
struct Camera {
    neo::CameraComponent *camera;
    Camera(float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        neo::GameObject *gameObject = &neo::Engine::createGameObject();
        neo::Engine::addComponent<neo::SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &neo::Engine::addComponent<neo::CameraComponent>(gameObject, fov, near, far);
        neo::Engine::addComponent<neo::CameraControllerComponent>(gameObject, ls, ms);
    }
};

struct Light {
    neo::GameObject *gameObject;
    neo::LightComponent *light;

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        gameObject = &neo::Engine::createGameObject();
        neo::Engine::addComponent<neo::SpatialComponent>(gameObject, pos);
        light = &neo::Engine::addComponent<neo::LightComponent>(gameObject, col, att);

        neo::Engine::addImGuiFunc("Light", [&]() {
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
    neo::GameObject *gameObject;

    Renderable(neo::Mesh *mesh, float amb, glm::vec3 diffuse, glm::vec3 specular) {
        gameObject = &neo::Engine::createGameObject();
        neo::Engine::addComponent<neo::SpatialComponent>(gameObject, glm::vec3(0.f), glm::vec3(1.f));
        neo::Engine::addComponent<neo::MeshComponent>(gameObject, mesh);
        neo::Engine::addComponent<neo::renderable::PhongRenderable>(gameObject);
        neo::Engine::addComponent<neo::MaterialComponent>(gameObject, amb, diffuse, specular);

        neo::Engine::addImGuiFunc("neo::Mesh", [&]() {
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
    neo::Engine::init("Base", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    Renderable(neo::Library::getMesh("cube"), 0.2f, glm::vec3(1.f, 0.f, 1.f), glm::vec3(1.f));

    /* Systems - order matters! */
    neo::Engine::addSystem<neo::CameraControllerSystem>();
    neo::Engine::initSystems();

    /* Init renderer */
    neo::Renderer::init("shaders/", camera.camera);
    neo::Renderer::addSceneShader<neo::PhongShader>();

    /* Attach ImGui panes */
    neo::Engine::addDefaultImGuiFunc();

    /* Run */
    neo::Engine::run();
    return 0;
}