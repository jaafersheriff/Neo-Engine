#include <Engine.hpp>

#include "PerspectiveUpdateSystem.hpp"

#include "Shader/ShadowCasterShader.hpp"
#include "Shader/PhongShadowShader.hpp"
#include "Shader/LineShader.hpp"
#include "Shader/WireFrameShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

static constexpr int shadowMapSize = 2048;

/* Game object definitions */
struct Camera {
    GameObject *gameObject;
    CameraComponent *camera;

    Camera(float fov, float near, float far, float ar, glm::vec3 pos) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov, ar);
    }
};

struct Light {
    Light(glm::vec3 position, bool attachCube = true) {
        // Light object
        auto lightObject = &Engine::createGameObject();
        auto& spatial = Engine::addComponent<SpatialComponent>(lightObject, position, glm::vec3(1.f));
        spatial.setLookDir(glm::vec3(0.f, -0.5f, 0.7f));
        Engine::addComponent<LightComponent>(lightObject, glm::vec3(1.f), glm::vec3(0.4f, 0.2f, 0.f));

        // Shadow camera object
        auto cameraObject = &Engine::createGameObject();
        Engine::addComponentAs<OrthoCameraComponent, CameraComponent>(cameraObject, -2.f, 2.f, -4.f, 2.f, 0.1f, 5.f);
        Engine::addComponent<SpatialComponent>(cameraObject, position, glm::vec3(1.f));
        Engine::addComponent<FrustumComponent>(cameraObject);
        Engine::addComponent<LineMeshComponent>(cameraObject, glm::vec3(1.f, 0.f, 1.f));
        Engine::addComponent<ShadowCameraComponent>(cameraObject);

        Engine::addImGuiFunc("Light", []() {
            auto light = Engine::getSingleComponent<LightComponent>();
            auto spatial = light->getGameObject().getComponentByType<SpatialComponent>();
            light->imGuiEditor();
            glm::vec3 lookdir = spatial->getLookDir();
            ImGui::SliderFloat3("lookdir", &lookdir[0], -1.f, 1.f);
            spatial->setLookDir(lookdir);
        });
    }
};

struct Renderable {
    GameObject *gameObject;

    Renderable(Mesh *mesh, glm::vec3 position = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f), glm::vec3 rotation = glm::vec3(0.f)) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<MeshComponent>(gameObject, mesh);
        Engine::addComponent<SpatialComponent>(gameObject, position, scale, rotation);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "FrustaFitting";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera sceneCamera(45.f, 1.f, 100.f, Window::getAspectRatio(), glm::vec3(0, 0.6f, 5));
    Engine::addComponent<CameraControllerComponent>(sceneCamera.gameObject, 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(sceneCamera.gameObject);
    
    // Perspective camera
    Camera mockCamera(50.f, 0.1f, 5.f, 1.f, glm::vec3(0.f, 2.f, -0.f));
    auto* line = &Engine::addComponent<LineMeshComponent>(mockCamera.gameObject, glm::vec3(0.f, 1.f, 1.f));
    Engine::addComponent<FrustumComponent>(mockCamera.gameObject);

    // Ortho camera, shadow camera, light
    Light light(glm::vec3(10.f, 20.f, 0.f), true);

    // Renderable
    for (int i = 0; i < 30; i++) {
        Renderable sphere(Util::genRandomBool() ? Library::getMesh("cube") : Library::getMesh("sphere"), glm::vec3(Util::genRandom(-10.f, 10.f), Util::genRandom(0.5f, 1.f), Util::genRandom(-10.f, 10.f)), glm::vec3(0.5f));
        Engine::addComponent<renderable::ShadowCasterRenderable>(sphere.gameObject);
        Engine::addComponent<renderable::PhongShadowRenderable>(sphere.gameObject);
        Engine::addComponent<MaterialComponent>(sphere.gameObject, 0.3f, Util::genRandomVec3(), glm::vec3(1.f), 20.f);
    }

    /* Ground plane */
    Renderable receiver(Library::getMesh("quad"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(50.f), glm::vec3(-1.56f, 0, 0));
    Engine::addComponent<MaterialComponent>(receiver.gameObject, 0.2f, glm::vec3(0.7f), glm::vec3(1.f), 20.f);
    Engine::addComponent<renderable::PhongShadowRenderable>(receiver.gameObject);

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>(); // Update camera
    Engine::addSystem<FrustumSystem>(); // Calculate original frusta bounds
    Engine::addSystem<FrustaFittingSystem>(); // Fit one frusta into another
    Engine::addSystem<FrustumToLineSystem>(); // Create line mesh
    auto& perspectiveUpdate = Engine::addSystem<PerspectiveUpdateSystem>(); // Update mock perspective camera

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addPreProcessShader<ShadowCasterShader>(shadowMapSize);
    Renderer::addSceneShader<PhongShadowShader>();
    Renderer::addSceneShader<LineShader>();

    /* Attach ImGui panes */
    Engine::addImGuiFunc("SceneCamera", [&]() {
        if (ImGui::Button("Set scene")) {
            Engine::removeComponent<MainCameraComponent>(*mockCamera.gameObject->getComponentByType<MainCameraComponent>());
            Engine::removeComponent(*mockCamera.gameObject->getComponentByType<CameraControllerComponent>());
            Engine::addComponent<MainCameraComponent>(sceneCamera.gameObject);
            Engine::addComponent<CameraControllerComponent>(sceneCamera.gameObject, 0.4f, 7.f);
        }
        if (ImGui::Button("Set perspective")) {
            Engine::removeComponent<MainCameraComponent>(*sceneCamera.gameObject->getComponentByType<MainCameraComponent>());
            Engine::removeComponent(*sceneCamera.gameObject->getComponentByType<CameraControllerComponent>());
            Engine::addComponent<MainCameraComponent>(mockCamera.gameObject);
            Engine::addComponent<CameraControllerComponent>(mockCamera.gameObject, 0.4f, 7.f);
        }

    });
    Engine::addImGuiFunc("PerspectiveCamera", [&]() {
        auto spatial = mockCamera.gameObject->getComponentByType<SpatialComponent>();
        auto camera = dynamic_cast<PerspectiveCameraComponent*>(mockCamera.camera);
        ImGui::Checkbox("Auto update", &perspectiveUpdate.mUpdatePerspective);
        spatial->imGuiEditor();
        camera->imGuiEditor();
    });

    /* Run */
    Engine::run();
    return 0;
}
