#include <Engine.hpp>

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
        GameObject *gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
        Engine::addComponent<CameraControllerComponent>(gameObject, ls, ms);
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
        gameObject = &Engine::createGameObject();
        camSpatial = &Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(2.f), glm::mat3(glm::rotate(glm::mat4(1.f), 0.6f, glm::vec3(1, 0, 0))));
        light = &Engine::addComponent<LightComponent>(gameObject, col, att);
        Engine::addComponent<MeshComponent>(gameObject, Library::getMesh("cube"));
        Engine::addComponent<renderable::PhongRenderable>(gameObject);
        Engine::addComponent<MaterialComponent>(gameObject, 1.f, glm::vec3(1.f));
        camera = &Engine::addComponentAs<OrthoCameraComponent, CameraComponent>(gameObject, -1.f, 1000.f, -100.f, 100.f, -100.f, 100.f);
        sin = &Engine::addComponent<SinTranslateComponent>(gameObject, glm::vec3(0.f, 0.f, 20.f), camSpatial->getPosition());

        Engine::addComponent<ShadowCameraComponent>(gameObject);

        // Separate game object for look at
        gameO = &Engine::createGameObject();
        lookAtSpatial = &Engine::addComponent<SpatialComponent>(gameO, glm::vec3(0.f), glm::vec3(1.f));
        Engine::addComponent<MeshComponent>(gameO, Library::getMesh("cube"));
        Engine::addComponent<renderable::WireframeRenderable>(gameO);
        Engine::addComponent<LookAtCameraReceiver>(gameO);

        Engine::addImGuiFunc("Light", [&]() {
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
            auto lookDir = camSpatial->getLookDir();
            ImGui::Text("Look at dir : %0.2f, %0.2f, %0.2f", lookDir.x, lookDir.y, lookDir.z);

            glm::vec2 nearFar = camera->getNearFar();
            if (ImGui::SliderFloat("Near", &nearFar.x, 0.f, 1.f) ||
                ImGui::SliderFloat("Far", &nearFar.y, 100.f, 1000.f)) {
                camera->setNearFar(nearFar.x, nearFar.y);
            }

            if (auto shadowCamera = dynamic_cast<OrthoCameraComponent*>(camera)) {
                glm::vec2 h = shadowCamera->getHorizontalBounds();
                glm::vec2 v = shadowCamera->getVerticalBounds();
                if (ImGui::SliderFloat2("Horizontal", glm::value_ptr(h), -50.f, 50.f)) {
                    shadowCamera->setOrthoBounds(h, v);
                }
                if (ImGui::SliderFloat2("Vertical", glm::value_ptr(v), -50.f, 50.f)) {
                    shadowCamera->setOrthoBounds(h, v);
                }

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
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, scale, ori);
        Engine::addComponent<MeshComponent>(gameObject, mesh);
    }

    void attachImGui(std::string name) {
        Engine::addImGuiFunc(name, [&]() {
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
    Engine::init("Shadows", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 20.f);
    Light light(glm::vec3(37.5f, 37.5f, 11.8f), glm::vec3(1.f), glm::vec3(0.6, 0.04, 0.f));

    // Spheres and blocks 
    for (int i = 0; i < 50; i++) {
        Renderable r(
            Util::genRandomBool() ? Library::getMesh("cube") : Library::getMesh("sphere"),
            glm::vec3(Util::genRandom(-45.f, 45.f), Util::genRandom(2.f, 5.f), Util::genRandom(-45.f, 45.f)),
            glm::vec3(Util::genRandom(5.f)));
        Engine::addComponent<renderable::ShadowCasterRenderable>(r.gameObject);
        Engine::addComponent<renderable::PhongShadowRenderable>(r.gameObject);
        Engine::addComponent<MaterialComponent>(r.gameObject, 0.3f, Util::genRandomVec3(), glm::vec3(1.f), 20.f);
    }

    // Terrain receiver 
    Renderable receiver(Library::getMesh("quad"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(100.f), glm::mat3(glm::rotate(glm::mat4(1.f), -1.56f, glm::vec3(1, 0, 0))));
    Engine::addComponent<MaterialComponent>(receiver.gameObject, 0.2f, glm::vec3(0.7f), glm::vec3(1.f), 20.f);
    Engine::addComponent<renderable::PhongShadowRenderable>(receiver.gameObject);
    receiver.attachImGui("Receiver");

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<LookAtCameraSystem>();
    Engine::addSystem<SinTranslateSystem>();
    Engine::initSystems();

    /* Init renderer */
    Renderer::init("shaders/", camera.camera);
    Renderer::addPreProcessShader<ShadowCasterShader>(2048);
    Renderer::addSceneShader<LineShader>();
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<PhongShadowShader>();
    Renderer::addSceneShader<WireframeShader>();

    /* Attach ImGui panes */
    Engine::addDefaultImGuiFunc();
    Engine::addImGuiFunc("Shadow Camera", [&]() {
        static bool useLightCam = false;
        if (ImGui::Button("Switch Camera")) {
            useLightCam = !useLightCam;
            Renderer::setDefaultCamera(useLightCam ? light.camera : camera.camera);
        }
    });

    /* Run */
    Engine::run();

    return 0;
}