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
    
    Camera mockCamera(50.f, 0.1f, 5.f, glm::vec3(0.f, 1.f, 0.f));
    auto* line = &Engine::addComponent<LineComponent>(mockCamera.gameObject, glm::vec3(1, 0, 1));
    Engine::addComponent<renderable::LineMeshComponent>(mockCamera.gameObject, line);
    Engine::addComponent<FrustaBoundsComponent>(mockCamera.gameObject);
    Engine::addComponent<MockPerspectiveComponent>(mockCamera.gameObject);

    GameObject* go = &Engine::createGameObject();
    Engine::addComponent<SpatialComponent>(go, glm::vec3(1.f, 1.f, 0.f), glm::vec3(1.f));
    Engine::addComponent<CameraComponent>(go, -2.f, 2.f, -4.f, 2.f, 0.1f, 5.f);
    Engine::addComponent<renderable::LineMeshComponent>(go, &Engine::addComponent<LineComponent>(go, glm::vec3(0.f, 1.f, 1.f)));
    Engine::addComponent<FrustaBoundsComponent>(go);
    Engine::addComponent<MockOrthoComponent>(go);

    /* Ground plane */
    Renderable plane(Library::getMesh("quad"), glm::vec3(0.f), glm::vec3(15.f), glm::vec3(-Util::PI() / 2.f, 0.f, 0.f));
    Engine::addComponent<renderable::AlphaTestRenderable>(plane.gameObject);
    Engine::addComponent<DiffuseMapComponent>(plane.gameObject, Library::getTexture("grid.png"));

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>(); // Update camera
    Engine::addSystem<FrustaBoundsSystem>(); // Calculate original frusta bounds
    Engine::addSystem<FrustaFittingSystem>(); // Fit one frusta into another
    Engine::addSystem<CameraLineSystem>(); // Create line mesh
    Engine::initSystems();

    /* Init renderer */
    Renderer::init("shaders/", sceneCamera.camera);
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<AlphaTestShader>();
    Renderer::addSceneShader<LineShader>();

    /* Attach ImGui panes */
    Engine::addDefaultImGuiFunc();
    Engine::addImGuiFunc("Renderer", [&]() {
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

    /* Run */
    Engine::run();
    return 0;
}