#include <NeoEngine.hpp>

#include "CustomSystem.hpp"

#include "Shader/LineShader.hpp"
#include "Shader/DiffuseShader.hpp"
#include "ShadowCasterShader.hpp"
#include "ShadowReceiverShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include "util/Util.hpp"

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
    RenderableComponent *renderable;
    Material material = Material(1.f, glm::vec3(1.f));
    CameraComponent *camera;

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        gameObject = &NeoEngine::createGameObject();
        auto spatial = &NeoEngine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(2.f), glm::mat3(glm::rotate(glm::mat4(1.f), 0.6f, glm::vec3(1, 0, 0))));
        light = &NeoEngine::addComponent<LightComponent>(gameObject, col, att);
        renderable = &NeoEngine::addComponent<RenderableComponent>(gameObject, Loader::getMesh("cube"));
        renderable->addShaderType<DiffuseShader>();
        NeoEngine::addComponent<MaterialComponent>(gameObject, &material);
        camera = &NeoEngine::addComponent<CameraComponent>(gameObject, -10.f, 10.f, -10.f, 10.f, -1.f, 1000.f);
        LineComponent *uLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(1.f, 0.f, 0.f));
        uLine->addNodes({ glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f) });
        LineComponent *vLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(0.f, 1.f, 0.f));
        vLine->addNodes({ glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f) });
        LineComponent *wLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(0.f, 0.f, 1.f));
        wLine->addNodes({ glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f) });
        LineComponent *lookLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(1.f, 1.f, 1.f));
        lookLine->addNodes({ glm::vec3(0.f), glm::normalize(-spatial->getW()) });
        NeoEngine::addComponent<LineRenderable>(gameObject, uLine);
        NeoEngine::addComponent<LineRenderable>(gameObject, vLine);
        NeoEngine::addComponent<LineRenderable>(gameObject, wLine);
        NeoEngine::addComponent<LineRenderable>(gameObject, lookLine);

        NeoEngine::addImGuiFunc("Light", [&]() {
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            static glm::vec3 lookAt = glm::vec3(0.f);
            ImGui::SliderFloat3("Position", glm::value_ptr(pos), -100.f, 100.f);
            ImGui::SliderFloat3("Look At", glm::value_ptr(lookAt), -100.f, 100.f);
            gameObject->getSpatial()->setPosition(pos);
            camera->setLookDir(lookAt - gameObject->getSpatial()->getPosition());
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
    Material material = Material(0.1f, glm::vec3(1,0,1));
    glm::vec3 rot = glm::vec3(0.f);

    Renderable(Mesh *mesh, glm::vec3 pos, glm::vec3 scale, glm::mat4 ori = glm::mat3()) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, scale, ori);
        renderable = &NeoEngine::addComponent<RenderableComponent>(gameObject, mesh);
        NeoEngine::addComponent<MaterialComponent>(gameObject, &material);
    }

    void attachImGui(std::string name) {
        NeoEngine::addImGuiFunc(name, [&]() {
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -10.f, 10.f)) {
                gameObject->getSpatial()->setPosition(pos);
            }
            float scale = gameObject->getSpatial()->getScale().x;
            if (ImGui::SliderFloat("Scale", &scale, 0.f, 10.f)) {
                gameObject->getSpatial()->setScale(glm::vec3(scale));
            }
            if (ImGui::SliderFloat3("Rotation", glm::value_ptr(rot), -4.f, 4.f)) {
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
    NeoEngine::init("Shadows", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 20.f);
    Light light(glm::vec3(37.5f, 37.5f, 11.8f), glm::vec3(1.f), glm::vec3(0.6, 0.04, 0.f));
    Renderable casterA(Loader::getMesh("mr_krab.obj"), glm::vec3(0.f, 3.f, 0.f), glm::vec3(3.f));
    casterA.renderable->addShaderType<ShadowCasterShader>();
    casterA.renderable->addShaderType<ShadowReceiverShader>();
    casterA.attachImGui("CasterA");
    Renderable casterB(Loader::getMesh("mr_krab.obj"), glm::vec3(0.f, 3.f, -5.f), glm::vec3(3.f));
    casterB.renderable->addShaderType<ShadowCasterShader>();
    casterB.renderable->addShaderType<ShadowReceiverShader>();
    casterB.attachImGui("CasterB");
    Renderable receiver(Loader::getMesh("quad"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(100.f), glm::mat3(glm::rotate(glm::mat4(1.f), -1.56f, glm::vec3(1, 0, 0))));
    receiver.material.diffuse = glm::vec3(0.7f);
    receiver.renderable->addShaderType<ShadowReceiverShader>();
    receiver.attachImGui("Receiver");

    /* Systems - order matters! */
    NeoEngine::addSystem<CustomSystem>();
    RenderSystem * renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/", camera.camera);
    NeoEngine::initSystems();

    /* Add shaders */
    renderSystem->addShader<ShadowCasterShader, ShaderTypes::PREPROCESS>("caster.vert", "caster.frag");
    renderSystem->addShader<LineShader>();
    renderSystem->addShader<DiffuseShader>();
    ShadowReceiverShader & receiverShader = renderSystem->addShader<ShadowReceiverShader>("receiver.vert", "receiver.frag");

    /* Attach ImGui panes */
    NeoEngine::addImGuiFunc("Stats", [&]() {
        ImGui::Text("FPS: %d", NeoEngine::FPS);
        ImGui::Text("dt: %0.4f", NeoEngine::timeStep);
        if (ImGui::Button("VSync")) {
            Window::toggleVSync();
        }
    });
    NeoEngine::addImGuiFunc("Shadow Map", [&]() {
        ImGui::SliderFloat("Bias", &receiverShader.bias, -0.005f, 0.005f);
        auto texture = Loader::getTexture("depthTexture");
        static float scale = 0.1f;
        ImGui::SliderFloat("Scale", &scale, 0.f, 1.f);
        ImGui::Image((ImTextureID)texture->textureId, ImVec2(scale * texture->width, scale * texture->height), ImVec2(0, 1), ImVec2(1, 0));
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