#include <Engine.hpp>

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/WireframeShader.hpp"
#include "Renderer/Shader/ShadowCasterShader.hpp"
#include "Renderer/Shader/PhongShadowShader.hpp"

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
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov, Window::getAspectRatio());
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
            ImGui::Separator();

            ImGui::Text("Light");
            light->imGuiEditor();
            ImGui::Separator();

            ImGui::Text("Shadow Camera");
            camSpatial->imGuiEditor();
            camera->imGuiEditor();
            ImGui::Separator();

            ImGui::Text("Look at position");
            lookAtSpatial->imGuiEditor();
            ImGui::Separator();
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
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Shadows";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 20.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

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

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<LookAtCameraSystem>();
    Engine::addSystem<SinTranslateSystem>();

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addPreProcessShader<ShadowCasterShader>(2048);
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<PhongShadowShader>();
    Renderer::addSceneShader<WireframeShader>();

    /* Attach ImGui panes */
    Engine::addImGuiFunc("Shadow Camera", [&]() {
        static bool useLightCam = false;
        if (ImGui::Button("Switch Camera")) {
            useLightCam = !useLightCam;
            if (useLightCam) {
                Engine::removeComponent<MainCameraComponent>(*camera.camera->getGameObject().getComponentByType<MainCameraComponent>());
                Engine::addComponent<MainCameraComponent>(&light.camera->getGameObject());
            }
            else {
                Engine::removeComponent<MainCameraComponent>(*light.camera->getGameObject().getComponentByType<MainCameraComponent>());
                Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());
            }
        }
    });

    /* Run */
    Engine::run();

    return 0;
}
