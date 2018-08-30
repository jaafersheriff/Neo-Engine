#include <NeoEngine.hpp>

#include "CustomSystem.hpp"

#include "SinMoveComponent.hpp"
#include "LookAtCameraController.hpp"

#include "Shader/LineShader.hpp"
#include "Shader/PhongShader.hpp"
#include "Shader/WireframeShader.hpp"
#include "ShadowCasterShader.hpp"
#include "ShadowReceiverShader.hpp"

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
    RenderableComponent *renderable;
    Material material = Material(1.f, glm::vec3(1.f));
    CameraComponent *camera;
    SpatialComponent *camSpatial;
    SinMoveComponent *sin = nullptr;

    GameObject *gameO;
    SpatialComponent *lookAtSpatial;

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        gameObject = &NeoEngine::createGameObject();
        camSpatial = &NeoEngine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(2.f), glm::mat3(glm::rotate(glm::mat4(1.f), 0.6f, glm::vec3(1, 0, 0))));
        light = &NeoEngine::addComponent<LightComponent>(gameObject, col, att);
        renderable = &NeoEngine::addComponent<RenderableComponent>(gameObject, Loader::getMesh("cube"));
        renderable->addShaderType<PhongShader>();
        NeoEngine::addComponent<MaterialComponent>(gameObject, &material);
        camera = &NeoEngine::addComponent<CameraComponent>(gameObject, -10.f, 10.f, -10.f, 10.f, -1.f, 1000.f);
        LineComponent *uLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(1.f, 0.f, 0.f));
        uLine->addNodes({ glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f) });
        LineComponent *vLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(0.f, 1.f, 0.f));
        vLine->addNodes({ glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f) });
        LineComponent *wLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(0.f, 0.f, 1.f));
        wLine->addNodes({ glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f) });
        LineComponent *lookLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(1.f, 1.f, 1.f));
        NeoEngine::addComponent<LineRenderable>(gameObject, uLine);
        NeoEngine::addComponent<LineRenderable>(gameObject, vLine);
        NeoEngine::addComponent<LineRenderable>(gameObject, wLine);
        sin = &NeoEngine::addComponent<SinMoveComponent>(gameObject, camSpatial->getPosition(), camSpatial->getPosition());

        // Separate game object for look at
        gameO = &NeoEngine::createGameObject();
        lookAtSpatial = &NeoEngine::addComponent<SpatialComponent>(gameO, glm::vec3(0.f), glm::vec3(1.f));
        auto cube = &NeoEngine::addComponent<RenderableComponent>(gameO, Loader::getMesh("cube"));
        cube->addShaderType<WireframeShader>();

        NeoEngine::addComponent<LookAtCameraController>(gameObject, camera, lookAtSpatial);

        NeoEngine::addImGuiFunc("Light", [&]() {
            ImGui::Text("CamReceivers: [%d, %d]", gameObject->getNumReceiverTypes(), gameObject->getNumReceivers());
            ImGui::Text("LookAtReceivers: [%d, %d]", gameO->getNumReceiverTypes(), gameO->getNumReceivers());
 
            glm::vec3 pos = camSpatial->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -100.f, 100.f)) {
                camSpatial->setPosition(pos);
            }
            if (sin) {
                ImGui::Checkbox("Sin", &sin->active);
                if (sin->active) {
                    ImGui::SliderFloat3("PosA", glm::value_ptr(sin->maxPos), -100.f, 100.f);
                    ImGui::SliderFloat3("PosB", glm::value_ptr(sin->minPos), -100.f, 100.f);
                }
            }
            pos = lookAtSpatial->getPosition();
            if (ImGui::SliderFloat3("look pos", glm::value_ptr(pos), -100.f, 100.f)) {
                lookAtSpatial->setPosition(pos);
            }
            auto lookDir = camera->getLookDir();
            ImGui::Text("Look at dir : %0.2f, %0.2f, %0.2f", lookDir.x, lookDir.y, lookDir.z);

            glm::vec2 h = camera->getHorizontalBounds();
            glm::vec2 v = camera->getVerticalBounds();
            if (ImGui::SliderFloat2("Horizontal", glm::value_ptr(h), -50.f, 50.f)) {
                camera->setOrthoBounds(h, v);
            }
            if (ImGui::SliderFloat2("Vertical", glm::value_ptr(v), -50.f, 50.f)) {
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
    Material *material;
    glm::vec3 rot = glm::vec3(0.f);

    Renderable(Mesh *mesh, glm::vec3 pos, glm::vec3 scale, glm::mat4 ori = glm::mat3()) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, scale, ori);
        renderable = &NeoEngine::addComponent<RenderableComponent>(gameObject, mesh);
        material = new Material;
        NeoEngine::addComponent<MaterialComponent>(gameObject, material);
    }

    void attachImGui(std::string name) {
        NeoEngine::addImGuiFunc(name, [&]() {
            ImGui::Text("CamReceivers: [%d, %d]", gameObject->getNumReceiverTypes(), gameObject->getNumReceivers());
            
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

    // Spheres and blocks 
    for (int i = 0; i < 50; i++) {
        Renderable r(
            Util::genRandomBool() ? Loader::getMesh("cube") : Loader::getMesh("sphere"),
            glm::vec3(Util::genRandom(-45.f, 45.f), Util::genRandom(2.f, 5.f), Util::genRandom(-45.f, 45.f)),
            glm::vec3(Util::genRandom(5.f)));
        r.renderable->addShaderType<ShadowCasterShader>();
        r.renderable->addShaderType<ShadowReceiverShader>();
        r.material->ambient = 0.5f;
        r.material->diffuse = Util::genRandomVec3();
    }


    // Terrain receiver 
    Renderable receiver(Loader::getMesh("quad"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(100.f), glm::mat3(glm::rotate(glm::mat4(1.f), -1.56f, glm::vec3(1, 0, 0))));
    receiver.material->diffuse = glm::vec3(0.7f);
    receiver.renderable->addShaderType<ShadowReceiverShader>();
    receiver.attachImGui("Receiver");

    /* Systems - order matters! */
    NeoEngine::addSystem<CustomSystem>();
    RenderSystem * renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/", camera.camera);
    NeoEngine::initSystems();

    /* Add shaders */
    renderSystem->addPreProcessShader<ShadowCasterShader>("caster.vert", "caster.frag");
    renderSystem->addSceneShader<LineShader>();
    renderSystem->addSceneShader<PhongShader>();
    ShadowReceiverShader & receiverShader = renderSystem->addSceneShader<ShadowReceiverShader>("receiver.vert", "receiver.frag");
    renderSystem->addSceneShader<WireframeShader>();

    /* Attach ImGui panes */
    NeoEngine::addImGuiFunc("Stats", [&]() {
        ImGui::Text("FPS: %d", Util::FPS);
        ImGui::Text("dt: %0.4f", Util::timeStep);
        if (ImGui::Button("VSync")) {
            Window::toggleVSync();
        }
    });
    NeoEngine::addImGuiFunc("Shadow Map", [&]() {
        ImGui::SliderFloat("Bias", &receiverShader.bias, 0.f, 0.005f, "%0.4f");
        ImGui::Checkbox("Dot bias", &receiverShader.useDotBias);
        ImGui::Checkbox("PCF", &receiverShader.usePCF);
        if (receiverShader.usePCF) {
            ImGui::SliderInt("PCF Size", &receiverShader.pcfSize, 0, 5);
        }
        const Texture * texture(renderSystem->framebuffers.find("depthMap")->second.get()->textures[0]);
        static float scale = 0.1f;
        ImGui::SliderFloat("Scale", &scale, 0.f, 1.f);
        ImGui::Image((ImTextureID)texture->textureId, ImVec2(scale * texture->width, scale * texture->height), ImVec2(0, 1), ImVec2(1, 0));
        static bool useLightCam = false;
        if (ImGui::Button("Switch Camera")) {
            useLightCam = !useLightCam;
            renderSystem->setDefaultCamera(useLightCam ? light.camera : camera.camera);
        }
    });

    /* Run */
    NeoEngine::run();

    return 0;
}