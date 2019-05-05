#include <NeoEngine.hpp>

#include "Shader/LineShader.hpp"
#include "Shader/PhongShader.hpp"
#include "Shader/WireframeShader.hpp"
#include "Shader/ShadowCasterShader.hpp"
#include "Shader/PhongShadowShader.hpp"

#include "LookAtCameraReceiver.hpp"
#include "LookAtCameraSystem.hpp"

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
    CameraComponent *camera;
    SpatialComponent *camSpatial;
    SinTranslateComponent *sin = nullptr;

    GameObject *gameO;
    SpatialComponent *lookAtSpatial;

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        gameObject = &NeoEngine::createGameObject();
        camSpatial = &NeoEngine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(2.f), glm::mat3(glm::rotate(glm::mat4(1.f), 0.6f, glm::vec3(1, 0, 0))));
        light = &NeoEngine::addComponent<LightComponent>(gameObject, col, att);
        NeoEngine::addComponent<MeshComponent>(gameObject, Library::getMesh("cube"));
        NeoEngine::addComponent<renderable::PhongRenderable>(gameObject);
        NeoEngine::addComponent<MaterialComponent>(gameObject, 1.f, glm::vec3(1.f));
        camera = &NeoEngine::addComponent<CameraComponent>(gameObject, -100.f, 100.f, -100.f, 100.f, -1.f, 1000.f);
        LineComponent *uLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(1.f, 0.f, 0.f));
        uLine->addNodes({ glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f) });
        LineComponent *vLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(0.f, 1.f, 0.f));
        vLine->addNodes({ glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f) });
        LineComponent *wLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(0.f, 0.f, 1.f));
        wLine->addNodes({ glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f) });
        LineComponent *lookLine = &NeoEngine::addComponent<LineComponent>(gameObject, glm::vec3(1.f, 1.f, 1.f));
        NeoEngine::addComponent<renderable::LineMeshComponent>(gameObject, uLine);
        NeoEngine::addComponent<renderable::LineMeshComponent>(gameObject, vLine);
        NeoEngine::addComponent<renderable::LineMeshComponent>(gameObject, wLine);
        sin = &NeoEngine::addComponent<SinTranslateComponent>(gameObject, glm::vec3(0.f, 0.f, 20.f), camSpatial->getPosition());

        NeoEngine::addComponent<ShadowCameraComponent>(gameObject);

        // Separate game object for look at
        gameO = &NeoEngine::createGameObject();
        lookAtSpatial = &NeoEngine::addComponent<SpatialComponent>(gameO, glm::vec3(0.f), glm::vec3(1.f));
        NeoEngine::addComponent<MeshComponent>(gameO, Library::getMesh("cube"));
        NeoEngine::addComponent<renderable::WireframeRenderable>(gameO);
        NeoEngine::addComponent<LookAtCameraReceiver>(gameO);

        NeoEngine::addImGuiFunc("Light", [&]() {
            ImGui::Text("CamReceivers: [%d, %d]", gameObject->getNumReceiverTypes(), gameObject->getNumReceivers());
            ImGui::Text("LookAtReceivers: [%d, %d]", gameO->getNumReceiverTypes(), gameO->getNumReceivers());
 
            glm::vec3 pos = camSpatial->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -100.f, 100.f)) {
                camSpatial->setPosition(pos);
            }
            if (sin) {
                ImGui::SliderFloat3("Base", glm::value_ptr(sin->mBasePosition), -100.f, 100.f);
                ImGui::SliderFloat3("Offset", glm::value_ptr(sin->mOffset), -100.f, 100.f);
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
            glm::vec2 nearFar = camera->getNearFar();
            if (ImGui::SliderFloat("Near", &nearFar.x, 0.f, 1.f) ||
               ImGui::SliderFloat("Far", &nearFar.y, 100.f, 1000.f)) {
                camera->setNearFar(nearFar.x, nearFar.y);
            }
            ImGui::SliderFloat3("Color", glm::value_ptr(light->mColor), 0.f, 1.f);
            ImGui::SliderFloat3("Attenuation", glm::value_ptr(light->mAttenuation), 0.f, 1.f);
        });
    }
};

struct Renderable {
    GameObject *gameObject;
    glm::vec3 rot = glm::vec3(0.f);

    Renderable(Mesh *mesh, glm::vec3 pos, glm::vec3 scale, glm::mat4 ori = glm::mat3()) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, scale, ori);
        NeoEngine::addComponent<MeshComponent>(gameObject, mesh);
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
            Util::genRandomBool() ? Library::getMesh("cube") : Library::getMesh("sphere"),
            glm::vec3(Util::genRandom(-45.f, 45.f), Util::genRandom(2.f, 5.f), Util::genRandom(-45.f, 45.f)),
            glm::vec3(Util::genRandom(5.f)));
        NeoEngine::addComponent<renderable::ShadowCasterRenderable>(r.gameObject);
        NeoEngine::addComponent<renderable::PhongShadowRenderable>(r.gameObject);
        NeoEngine::addComponent<MaterialComponent>(r.gameObject, 0.3f, Util::genRandomVec3(), glm::vec3(1.f), 20.f);
    }

    // Terrain receiver 
    Renderable receiver(Library::getMesh("quad"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(100.f), glm::mat3(glm::rotate(glm::mat4(1.f), -1.56f, glm::vec3(1, 0, 0))));
    NeoEngine::addComponent<MaterialComponent>(receiver.gameObject, 0.2f, glm::vec3(0.7f), glm::vec3(1.f), 20.f);
    NeoEngine::addComponent<renderable::PhongShadowRenderable>(receiver.gameObject);
    receiver.attachImGui("Receiver");

    /* Systems - order matters! */
    NeoEngine::addSystem<CameraControllerSystem>();
    NeoEngine::addSystem<LookAtCameraSystem>();
    NeoEngine::addSystem<SinTranslateSystem>();
    NeoEngine::initSystems();

    /* Init renderer */
    MasterRenderer::init("shaders/", camera.camera);
    MasterRenderer::addPreProcessShader<ShadowCasterShader>();
    MasterRenderer::addSceneShader<LineShader>();
    MasterRenderer::addSceneShader<PhongShader>();
    PhongShadowShader& receiverShader = MasterRenderer::addSceneShader<PhongShadowShader>();
    MasterRenderer::addSceneShader<WireframeShader>();

    /* Attach ImGui panes */
    NeoEngine::addDefaultImGuiFunc();
    NeoEngine::addImGuiFunc("Shadow Map", [&]() {
        ImGui::SliderFloat("Bias", &receiverShader.bias, 0.f, 0.005f, "%0.4f");
        ImGui::Checkbox("Dot bias", &receiverShader.useDotBias);
        ImGui::Checkbox("PCF", &receiverShader.usePCF);
        if (receiverShader.usePCF) {
            ImGui::SliderInt("PCF Size", &receiverShader.pcfSize, 0, 5);
        }
        static bool useLightCam = false;
        if (ImGui::Button("Switch Camera")) {
            useLightCam = !useLightCam;
            MasterRenderer::setDefaultCamera(useLightCam ? light.camera : camera.camera);
        }
    });

    /* Run */
    NeoEngine::run();

    return 0;
}