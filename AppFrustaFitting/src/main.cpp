#include <Engine.hpp>

#include "FrustaBoundsComponent.hpp"
#include "MockPerspectiveComponent.hpp"
#include "MockOrthoComponent.hpp"

#include "CameraLineSystem.hpp"
#include "FrustaBoundsSystem.hpp"
#include "FrustaFittingSystem.hpp"

#include "Shader/PhongShader.hpp"
#include "Shader/AlphaTestShader.hpp"
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
    
    Camera mockCamera(50.f, 0.f, 5.f, glm::vec3(0.f, 1.f, 0.f));
    auto* line = &Engine::addComponent<LineComponent>(mockCamera.gameObject, glm::vec3(1, 0, 1));
    Engine::addComponent<renderable::LineMeshComponent>(mockCamera.gameObject, line);
    Engine::addComponent<FrustaBoundsComponent>(mockCamera.gameObject);
    Engine::addComponent<MockPerspectiveComponent>(mockCamera.gameObject);
    
    GameObject* go = &Engine::createGameObject();
    Engine::addComponent<SpatialComponent>(go, glm::vec3(10.f, 10.f, 0.f));
    Engine::addComponent<CameraComponent>(go, -2.f, 2.f, -4.f, 2.f, 0.f, 5.f);
    Engine::addComponent<renderable::LineMeshComponent>(go, &Engine::addComponent<LineComponent>(go, glm::vec3(0.f, 1.f, 1.f)));
    Engine::addComponent<FrustaBoundsComponent>(go);
    Engine::addComponent<MockOrthoComponent>(go);
    // give ortho a source cube because it's hard to tell what's the near and far side
    Engine::addComponent<MeshComponent>(go, Library::getMesh("cube"));
    Engine::addComponent<renderable::PhongRenderable>(go);
    Engine::addComponent<MaterialComponent>(go, 0.5f, glm::vec3(0.f,1.f,1.f), glm::vec3(1.f));
    Engine::addComponent<renderable::WireframeRenderable>(go);

    /* Ground plane */
    Renderable plane(Library::getMesh("quad"), glm::vec3(0.f), glm::vec3(18.75f), glm::vec3(-Util::PI() / 2.f, 0.f, 0.f));
    Engine::addComponent<renderable::AlphaTestRenderable>(plane.gameObject);
    Engine::addComponent<DiffuseMapComponent>(plane.gameObject, Library::getTexture("grid.png"));

    auto& lightGO = Engine::createGameObject();
    Engine::addComponent<SpatialComponent>(&lightGO, glm::vec3(0.f, 1.f, 20.f), glm::vec3(1.f));
    Engine::addComponent<LightComponent>(&lightGO, glm::vec3(1.f), glm::vec3(0.6f, 0.2f, 0.01f));

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>(); // Update camera
    Engine::addSystem<FrustaBoundsSystem>(); // Calculate original frusta bounds
    auto& fitSystem = Engine::addSystem<FrustaFittingSystem>(); // Fit one frusta into another
    Engine::addSystem<CameraLineSystem>(); // Create line mesh
    Engine::initSystems();

    /* Init renderer */
    Renderer::init("shaders/", sceneCamera.camera);
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<AlphaTestShader>();
    Renderer::addSceneShader<LineShader>();
    Renderer::addSceneShader<WireframeShader>();

    /* Attach ImGui panes */
    Engine::addDefaultImGuiFunc();
    Engine::addImGuiFunc("SceneCamera", [&]() {
        if (ImGui::Button("Set scene")) {
            Renderer::setDefaultCamera(sceneCamera.camera);
        }
        if (ImGui::Button("Set perspective")) {
            Renderer::setDefaultCamera(mockCamera.camera);
        }
        if (ImGui::Button("Set ortho")) {
            Renderer::setDefaultCamera(go->getComponentByType<CameraComponent>());
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
            if (ImGui::SliderFloat("Near", &nearfar[0], 0.f, 2.f)) {
                camera->setNearFar(nearfar.x, nearfar.y);
            }
            if (ImGui::SliderFloat("Far", &nearfar[1], 2.f, 6.f)) {
                camera->setNearFar(nearfar.x, nearfar.y);
            }
        }
    });
    Engine::addImGuiFunc("OrthoCamera", [&]() {
        auto spatial = go->getSpatial();
        auto camera = go->getComponentByType<CameraComponent>();
        {
            glm::vec3 camPos = spatial->getPosition();
            ImGui::SliderFloat3("Position", &camPos[0], -20.f, 20.f);
            spatial->setPosition(camPos);
        }
        {
            ImGui::Checkbox("Auto update", &fitSystem.updateOrtho);
            if (!fitSystem.updateOrtho) {
                glm::vec3 lookDir = camera->getLookDir();
                ImGui::SliderFloat3("Look", &lookDir[0], -1.f, 1.f);
                camera->setLookDir(lookDir);
            }
        }
        {
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
    });

    /* Run */
    Engine::run();
    return 0;
}