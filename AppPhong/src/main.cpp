#include <NeoEngine.hpp>

#include "Shader/PhongShader.hpp"
#include "Shader/WireframeShader.hpp"

using namespace neo;

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
    RenderableComponent *cube;

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos);
        light = &NeoEngine::addComponent<LightComponent>(gameObject, col, att);
        cube = &NeoEngine::addComponent<RenderableComponent>(gameObject, Loader::getMesh("cube"));
        cube->addShaderType<WireframeShader>();

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
    RenderableComponent *renderComp;

    Renderable(Mesh *mesh, Material *mat, Texture *tex, glm::vec3 p, float s = 1.f, glm::mat3 o = glm::mat3()) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, p, glm::vec3(s), o);
        renderComp = &NeoEngine::addComponent<RenderableComponent>(gameObject, mesh);
        renderComp->addShaderType<PhongShader>();
        NeoEngine::addComponent<MaterialComponent>(gameObject, mat);
        NeoEngine::addComponent<DiffuseMapComponent>(gameObject, tex);
    }
};

int main() {
    NeoEngine::init("Phong Rendering", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 0.01f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

    std::vector<Renderable *> renderables;
    for (int x = -2; x < 3; x++) {
        for (int z = 0; z < 10; z++) {
            renderables.push_back(
                new Renderable(
                    Loader::getMesh("mr_krab.obj", true), 
                    Loader::getMaterial("krab"),
                    Loader::getTexture("mr_krab.png"),
                    glm::vec3(x*2, 0, z*2))
            );
        }
    }

    /* Systems - order matters! */
    NeoEngine::addSystem<CameraControllerSystem>();
    NeoEngine::initSystems();

    /* Init renderer */
    MasterRenderer::init("shaders/", camera.cameraComp);
    MasterRenderer::addSceneShader<PhongShader>();
    MasterRenderer::addSceneShader<WireframeShader>();

    /* Attach ImGui panes */
    NeoEngine::addDefaultImGuiFunc();

    /* Run */
    NeoEngine::run();

    return 0;
}