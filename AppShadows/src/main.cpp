#include <NeoEngine.hpp>

#include "CustomSystem.hpp"

#include "Shader/LineShader.hpp"
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
    Material material = Material(1.f, glm::vec3(1.f, 0.f, 1.f));
    CameraComponent *camera;

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(2.f));
        light = &NeoEngine::addComponent<LightComponent>(gameObject, col, att);
        renderable = &NeoEngine::addComponent<RenderableComponent>(gameObject, Loader::getMesh("cube"));
        renderable->addShaderType<DiffuseShader>();
        NeoEngine::addComponent<MaterialComponent>(gameObject, &material);
        camera = &NeoEngine::addComponent<CameraComponent>(gameObject, -20.f, 20.f, -20.f, 20.f, 1.f, 1000.f);
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

        NeoEngine::addImGuiFunc("Light", [&]() {
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -50.f, 50.f)) {
                gameObject->getSpatial()->setPosition(pos);
            }
            static glm::vec3 rot(0.f);
            if (ImGui::SliderFloat3("Rotation", glm::value_ptr(rot), -Util::PI(), Util::PI())) {
                glm::mat4 R;
                R = glm::rotate(glm::mat4(1.f), rot.x, glm::vec3(1, 0, 0));
                R *= glm::rotate(glm::mat4(1.f), rot.y, glm::vec3(0, 1, 0));
                R *= glm::rotate(glm::mat4(1.f), rot.z, glm::vec3(0, 0, 1));
                gameObject->getSpatial()->setOrientation(glm::mat3(R));
            }
            glm::vec2 h = camera->getHorizontalBounds();
            glm::vec2 v = camera->getVerticalBounds();
            if (ImGui::SliderFloat2("Horizontal", glm::value_ptr(h), 5.f, 50.f)) {
                camera->setOrthoBounds(h, v);
            }
            if (ImGui::SliderFloat2("Vertical", glm::value_ptr(v), 5.f, 50.f)) {
                camera->setOrthoBounds(h, v);
            }
            float near = camera->getNear();
            float far = camera->getFar();
            if (ImGui::SliderFloat("Near", &near, 0.f, 1.f)) {
                camera->setNearFar(near, far);
            }
            if (ImGui::SliderFloat("Far", &far, 100.f, 1000.f)) {
                camera->setNearFar(near, far);
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
    Camera camera(45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 20.f);
    Light light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    for (int i = 0; i < 100; i++) {
        glm::vec3 pos = glm::vec3(Util::genRandom(-2.f, 2.f), i * 2, 0.f);
        Renderable caster = Renderable(Loader::getMesh("mr_krab.obj"), pos, glm::vec3(1.f));
        caster.material.diffuse = Util::genRandomVec3();
        caster.renderable->addShaderType<ShadowCasterShader>();
        caster.renderable->addShaderType<DiffuseShader>();
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
    renderSystem->addShader<LineShader>();
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
        static bool useLightCam = false;
        if (ImGui::Button("Switch Camera")) {
            useLightCam = !useLightCam;
            if (useLightCam) {
                renderSystem->setDefaultCamera(light.camera);
            }
            else {
                renderSystem->setDefaultCamera(camera.camera);
            }
        }
    });



    /* Run */
    NeoEngine::run();

    return 0;
}