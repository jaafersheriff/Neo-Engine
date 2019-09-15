#include <Engine.hpp>

#include "FrustumBoundsComponent.hpp"
#include "MockPerspectiveComponent.hpp"
#include "MockOrthoComponent.hpp"

#include "FrustumBoundsToLineSystem.hpp"
#include "FrustaBoundsSystem.hpp"
#include "FrustaFittingSystem.hpp"

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

    Camera(float fov, float near, float far, glm::vec3 pos) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
    }
};

struct Light {
    Light(glm::vec3 position, bool attachCube = true) {
        auto lightObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(lightObject, position, glm::vec3(1.f));
        Engine::addComponent<LightComponent>(lightObject, glm::vec3(1.f), glm::vec3(0.4f, 0.2f, 0.f));

        auto cameraObject = &Engine::createGameObject();
        Engine::addComponent<MockOrthoComponent>(cameraObject);
        Engine::addComponent<SpatialComponent>(cameraObject, position, glm::vec3(1.f));
        Engine::addComponentAs<OrthoCameraComponent, CameraComponent>(cameraObject, -2.f, 2.f, -4.f, 2.f, 0.1f, 5.f);
        Engine::addComponent<FrustumBoundsComponent>(cameraObject);
        Engine::addComponent<renderable::LineMeshComponent>(cameraObject, &Engine::addComponent<LineComponent>(cameraObject), glm::vec3(0.f, 1.f, 1.f));
        Engine::addComponent<ShadowCameraComponent>(cameraObject);
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
    Engine::init("FrustaFitting", "res/", 1280, 720);

    /* Game objects */
    Camera sceneCamera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5));
    Engine::addComponent<CameraControllerComponent>(sceneCamera.gameObject, 0.4f, 7.f);
    
    // Perspective camera
    Camera mockCamera(50.f, 0.1f, 5.f, glm::vec3(0.f, 2.f, -0.f));
    auto* line = &Engine::addComponent<LineComponent>(mockCamera.gameObject);
    Engine::addComponent<renderable::LineMeshComponent>(mockCamera.gameObject, line, glm::vec3(1, 0, 1));
    Engine::addComponent<FrustumBoundsComponent>(mockCamera.gameObject);
    Engine::addComponent<MockPerspectiveComponent>(mockCamera.gameObject);

    // Ortho camera, shadow camera, light
    Light light(glm::vec3(10.f, 20.f, 0.f), true);

    // Renderable
    for (int i = 0; i < 10; i++) {
        Renderable sphere(Util::genRandomBool() ? Library::getMesh("cube") : Library::getMesh("sphere"), glm::vec3(Util::genRandom(-4.f, 4.f), Util::genRandom(0.5f, 1.f), Util::genRandom(-4.f, 4.f)), glm::vec3(0.5f));
        Engine::addComponent<renderable::ShadowCasterRenderable>(sphere.gameObject);
        Engine::addComponent<renderable::PhongShadowRenderable>(sphere.gameObject);
        Engine::addComponent<MaterialComponent>(sphere.gameObject, 0.3f, Util::genRandomVec3(), glm::vec3(1.f), 20.f);
    }

    /* Ground plane */
    Renderable receiver(Library::getMesh("quad"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(10.f), glm::vec3(-1.56f, 0, 0));
    Engine::addComponent<MaterialComponent>(receiver.gameObject, 0.2f, glm::vec3(0.7f), glm::vec3(1.f), 20.f);
    Engine::addComponent<renderable::PhongShadowRenderable>(receiver.gameObject);

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>(); // Update camera
    Engine::addSystem<FrustaBoundsSystem>(); // Calculate original frusta bounds
    auto& fitSystem = Engine::addSystem<FrustaFittingSystem>(1.f / shadowMapSize); // Fit one frusta into another
    Engine::addSystem<FrustumBoundsToLineSystem>(); // Create line mesh

    /* Init renderer */
    Renderer::init("shaders/", sceneCamera.camera);
    Renderer::addPreProcessShader<ShadowCasterShader>(shadowMapSize);
    Renderer::addSceneShader<PhongShadowShader>();
    Renderer::addSceneShader<LineShader>();

    /* Attach ImGui panes */
    Engine::addDefaultImGuiFunc();
    Engine::addImGuiFunc("SceneCamera", [&]() {
        if (ImGui::Button("Set scene")) {
            Renderer::setDefaultCamera(sceneCamera.camera);
            Engine::addComponent<CameraControllerComponent>(sceneCamera.gameObject, 0.4f, 7.f);
            Engine::removeComponent(*mockCamera.gameObject->getComponentByType<CameraControllerComponent>());
        }
        if (ImGui::Button("Set perspective")) {
            Renderer::setDefaultCamera(mockCamera.camera);
            Engine::addComponent<CameraControllerComponent>(mockCamera.gameObject, 0.4f, 7.f);
            Engine::removeComponent(*sceneCamera.gameObject->getComponentByType<CameraControllerComponent>());
        }

    });
    Engine::addImGuiFunc("PerspectiveCamera", [&]() {
        auto spatial = mockCamera.gameObject->getComponentByType<SpatialComponent>();
        auto camera = dynamic_cast<PerspectiveCameraComponent*>(mockCamera.camera);
        {
            glm::vec3 camPos = spatial->getPosition();
            ImGui::SliderFloat3("Position", &camPos[0], -20.f, 20.f);
            spatial->setPosition(camPos);
        }
        {
            ImGui::Checkbox("Auto update", &fitSystem.updatePerspective);
            if (!fitSystem.updatePerspective) {
                glm::vec3 lookDir = spatial->getLookDir();
                ImGui::SliderFloat3("Look", &lookDir[0], -1.f, 1.f);
                spatial->setLookDir(lookDir);
            }
        }
        {
            float fov = camera->getFOV();
            glm::vec2 nearfar = camera->getNearFar();
            if (ImGui::SliderFloat("FOV", &fov, 15.f, 110.f)) {
                camera->setFOV(fov);
            }
            if (ImGui::SliderFloat("Near", &nearfar[0], 0.f, 3.f)) {
                camera->setNearFar(nearfar.x, nearfar.y);
            }
            if (ImGui::SliderFloat("Far", &nearfar[1], 2.f, 20.f)) {
                camera->setNearFar(nearfar.x, nearfar.y);
            }
        }
    });

    /* Run */
    Engine::run();
    return 0;
}