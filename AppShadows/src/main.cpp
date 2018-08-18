#include <NeoEngine.hpp>

#include "CustomSystem.hpp"

#include "Shader/DiffuseShader.hpp"
#include "ShadowCasterShader.hpp"
#include "ShadowReceiverShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include "util/Util.hpp"

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
    RenderableComponent *renderable;
    Material material = Material(1.f, glm::vec3(1.f));
    CameraComponent *camera;

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(2.f));
        light = &NeoEngine::addComponent<LightComponent>(gameObject, col, att);
        renderable = &NeoEngine::addComponent<RenderableComponent>(gameObject, Loader::getMesh("cube"));
        renderable->addShaderType<DiffuseShader>();
        NeoEngine::addComponent<MaterialComponent>(gameObject, &material);
        camera = &NeoEngine::addComponent<CameraComponent>(gameObject, 45.f, 1.f, 100.f);

        NeoEngine::addImGuiFunc("Light", [&]() {
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -50.f, 50.f)) {
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

    Renderable(Mesh *mesh, glm::vec3 pos, glm::vec3 scale) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, scale);
        renderable = &NeoEngine::addComponent<RenderableComponent>(gameObject, mesh);
        NeoEngine::addComponent<MaterialComponent>(gameObject, &material);
    }
};

int main() {
    NeoEngine::init("Shadows", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 20.f);
    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    for (int i = 0; i < 100; i++) {
        glm::vec3 pos = glm::vec3(Util::genRandom(-25.f, 25.f), Util::genRandom(0.f, 25.f), Util::genRandom(-25.f, 25.f));
        glm::vec3 scale = Util::genRandomVec3(0.2f, 3.5f);
        Renderable caster = Renderable(Loader::getMesh("cube"), pos, scale);
        caster.material.diffuse = Util::genRandomVec3();
        caster.renderable->addShaderType<ShadowCasterShader>();
        caster.renderable->addShaderType<ShadowReceiverShader>();
    }
    Renderable receiver(Loader::getMesh("cube"), glm::vec3(0.f, -0.5f, 0.f), glm::vec3(100.f, 0.f, 100.f));
    receiver.material.diffuse = glm::vec3(0.7f);
    receiver.renderable->addShaderType<ShadowReceiverShader>();

    /* Systems - order matters! */
    NeoEngine::addSystem<CustomSystem>();
    renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/", camera.camera);
    NeoEngine::initSystems();

    /* Add shaders */
    renderSystem->addShader<ShadowCasterShader, ShaderTypes::PREPROCESS>("caster.vert", "caster.frag");
    renderSystem->addShader<DiffuseShader>();
    renderSystem->addShader<ShadowReceiverShader>("receiver.vert", "receiver.frag");

    /* Attach ImGui panes */
    NeoEngine::addImGuiFunc("Stats", [&]() {
        ImGui::Text("FPS: %d", NeoEngine::FPS);
        ImGui::Text("dt: %0.4f", NeoEngine::timeStep);
        if (ImGui::Button("VSync")) {
            Window::toggleVSync();
        }
    });
    NeoEngine::addImGuiFunc("Shadow Map", [&]() {
        auto texture = Loader::getTexture("depthTexture");
        static float scale = 0.2f;
        ImGui::SliderFloat("Scale", &scale, 0.f, 1.f);
        ImGui::Image((ImTextureID)texture->textureId, ImVec2(scale * texture->width, scale * texture->height));
    });



    /* Run */
    NeoEngine::run();

    return 0;
}