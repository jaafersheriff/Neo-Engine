#include "Engine/Engine.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/AlphaTestShader.hpp"
#include "Renderer/Shader/LineShader.hpp"
#include "Renderer/Shader/GammaCorrectShader.hpp"

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
    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        auto& gameObject = Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(&gameObject, pos);
        Engine::addComponent<LightComponent>(&gameObject, col, att);

        Engine::addImGuiFunc("Light", [&]() {
            if (auto light = Engine::getSingleComponent<LightComponent>()) {
                light->imGuiEditor();
            }
        });
    }
};

struct Renderable {
    GameObject *gameObject;

    Renderable(Mesh *mesh, glm::vec3 position = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f), glm::vec3 rotation = glm::vec3(0.f)) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<MeshComponent>(gameObject, *mesh);
        Engine::addComponent<SpatialComponent>(gameObject, position, scale, rotation);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "FrustaFitting";
    config.APP_RES = "res/";
    config.attachEditor = false;
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

    /* Cube object */
    for (int i = 0; i < 30; i++) {
        Renderable r(Library::getMesh("sphere"), glm::vec3(Util::genRandom(-7.5, 7.5), 0.f, Util::genRandom(-7.5, 7.5)));
        Material material;
        material.mAmbient = glm::vec3(0.2f);
        material.mDiffuse = glm::vec3(1.f, 1.f, 1.f);
        material.mShininess = 20.f;
        Engine::addComponent<renderable::PhongRenderable>(r.gameObject, *Library::getTexture("black"), material);
        Engine::addComponent<BoundingBoxComponent>(r.gameObject, *Library::getMesh("sphere"));
        Engine::addComponent<SelectableComponent>(r.gameObject);
    }

    /* Ground plane */
    {
        Renderable plane(Library::getMesh("quad"), glm::vec3(0.f), glm::vec3(15.f), glm::vec3(-Util::PI / 2.f, 0.f, 0.f));
        Engine::addComponent<renderable::AlphaTestRenderable>(plane.gameObject, *Library::loadTexture("grid.png"));
    }

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<MouseRaySystem>(true);
    Engine::addSystem<SelectingSystem>(
        "Selecter System",
        20, 
        100.f,
        // Decide to remove selected components
        [](SelectedComponent* selected) {
            return false;
        },
        // Reset operation for unselected components
        [](SelectableComponent* selectable) {
            if (auto renderable = selectable->getGameObject().getComponentByType<renderable::PhongRenderable>()) {
                renderable->mMaterial.mDiffuse = glm::vec3(1.f);
            }
        },
        // Operate on selected components
        [](SelectedComponent* selected, const MouseRayComponent*, float) {
            if (auto renderable = selected->getGameObject().getComponentByType<renderable::PhongRenderable>()) {
                renderable->mMaterial.mDiffuse = glm::vec3(1.f, 0.f, 0.f);
            }
        },
        // imgui editor
        [](std::vector<SelectedComponent*> selectedComps) {
            glm::vec3 scale = selectedComps[0]->getGameObject().getComponentByType<SpatialComponent>()->getScale();
            ImGui::SliderFloat3("Scale", &scale[0], 0.f, 3.f);
            for (auto selected : selectedComps) {
                selected->getGameObject().getComponentByType<SpatialComponent>()->imGuiEditor();
            }
        }
    );

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<AlphaTestShader>();
    Renderer::addSceneShader<LineShader>();
    Renderer::addPostProcessShader<GammaCorrectShader>();

    /* Attach ImGui panes */

    /* Run */
    Engine::run();
    return 0;
}
