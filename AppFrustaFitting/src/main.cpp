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

/* Game object definitions */
struct Camera {
    GameObject *gameObject;
    CameraComponent *camera;

    Camera(float fov, float near, float far, glm::vec3 pos) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &Engine::addComponent<CameraComponent>(gameObject, fov, near, far);
    }
};

struct Light {
    GameObject* gameObject;
    CameraComponent *camera;

    Light(glm::vec3 position, bool attachCube = true) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, position, glm::vec3(1.f));
        camera = &Engine::addComponent<CameraComponent>(gameObject, -2.f, 2.f, -4.f, 2.f, 0.1f, 5.f);
        Engine::addComponent<renderable::LineMeshComponent>(gameObject, &Engine::addComponent<LineComponent>(gameObject, glm::vec3(0.f, 1.f, 1.f)));
        Engine::addComponent<FrustumBoundsComponent>(gameObject);
        Engine::addComponent<MockOrthoComponent>(gameObject, glm::length(position));
        Engine::addComponent<LightComponent>(gameObject, glm::vec3(1.f), glm::vec3(0.4f, 0.2f, 0.f));
        Engine::addComponent<ShadowCameraComponent>(gameObject);

        // give ortho a source cube because it's hard to tell what's the near and far side
        if (attachCube) {
            Engine::addComponent<MeshComponent>(gameObject, Library::getMesh("cube"));
            Engine::addComponent<renderable::WireframeRenderable>(gameObject, glm::vec3(0.f,1.f,1.f));
        }
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
    auto* line = &Engine::addComponent<LineComponent>(mockCamera.gameObject, glm::vec3(1, 0, 1));
    Engine::addComponent<renderable::LineMeshComponent>(mockCamera.gameObject, line);
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
    auto& fitSystem = Engine::addSystem<FrustaFittingSystem>(); // Fit one frusta into another
    Engine::addSystem<FrustumBoundsToLineSystem>(); // Create line mesh
    Engine::initSystems();

    /* Init renderer */
    Renderer::init("shaders/", sceneCamera.camera);
    Renderer::addPreProcessShader<ShadowCasterShader>();
    Renderer::addSceneShader<PhongShadowShader>();
    Renderer::addSceneShader<LineShader>();
    Renderer::addSceneShader<WireframeShader>();

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
        if (ImGui::Button("Set Ortho")) {
            Renderer::setDefaultCamera(light.camera);
            Engine::removeComponent(*sceneCamera.gameObject->getComponentByType<CameraControllerComponent>());
            Engine::removeComponent(*mockCamera.gameObject->getComponentByType<CameraControllerComponent>());
        }

    });
    Engine::addImGuiFunc("PerspectiveCamera", [&]() {
        auto spatial = mockCamera.gameObject->getSpatial();
        auto camera = mockCamera.camera;
        {
            glm::vec3 camPos = spatial->getPosition();
            ImGui::SliderFloat3("Position", &camPos[0], -20.f, 20.f);
            spatial->setPosition(camPos);
        }
        {
            ImGui::Checkbox("Auto update", &fitSystem.updatePerspective);
            if (!fitSystem.updatePerspective) {
                glm::vec3 lookDir = camera->getLookDir();
                ImGui::SliderFloat3("Look", &lookDir[0], -1.f, 1.f);
                camera->setLookDir(lookDir);
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
    Engine::addImGuiFunc("OrthoCamera", [&]() {
        auto spatial = light.gameObject->getSpatial();
        auto camera = light.camera;
        {
            ImGui::SliderFloat("Distance", &light.gameObject->getComponentByType<MockOrthoComponent>()->distance, 1.f, 75.f);
            glm::vec3 camPos = spatial->getPosition();
            if (ImGui::SliderFloat3("Position", &camPos[0], -10.f, 10.f)) {
                spatial->setPosition(camPos);
            }
        }
        {
            glm::vec3 lookDir = camera->getLookDir();
            ImGui::SliderFloat3("LookDir", &lookDir[0], -1.f, 1.f);
            camera->setLookDir(lookDir);
        }
        {
            ImGui::Checkbox("Auto update", &fitSystem.updateOrtho);
            if (!fitSystem.updateOrtho) {
                glm::vec2 nearfar = camera->getNearFar();
                glm::vec2 hBounds = camera->getHorizontalBounds();
                glm::vec2 vBounds = camera->getVerticalBounds();
                if (ImGui::SliderFloat("Near", &nearfar[0], 0.f, 2.f)) {
                    camera->setNearFar(nearfar.x, nearfar.y);
                }
                if (ImGui::SliderFloat("Far", &nearfar[1], 2.f, 6.f)) {
                    camera->setNearFar(nearfar.x, nearfar.y);
                }
                if (ImGui::SliderFloat2("Horizontal Bounds", &hBounds[0], -5.f, 5.f)) {
                    camera->setOrthoBounds(hBounds, vBounds);
                }
                if (ImGui::SliderFloat2("Vertical Bounds", &vBounds[0], -5.f, 5.f)) {
                    camera->setOrthoBounds(hBounds, vBounds);
                }
            }
            else {
                if (ImGui::RadioButton("Dumb", fitSystem.method == FrustaFittingSystem::Method::Dumb)) {
                    fitSystem.method = FrustaFittingSystem::Method::Dumb;
                }
                if (ImGui::RadioButton("Naive", fitSystem.method == FrustaFittingSystem::Method::Naive)) {
                    fitSystem.method = FrustaFittingSystem::Method::Naive;
                }
                if (ImGui::RadioButton("A", fitSystem.method == FrustaFittingSystem::Method::A)) {
                    fitSystem.method = FrustaFittingSystem::Method::A;
                }
            }
        }
    });

    /* Run */
    Engine::run();
    return 0;
}