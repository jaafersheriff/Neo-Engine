#include <Engine.hpp>

#include "MetaballsMeshComponent.hpp"
#include "MetaballsSystem.hpp"
#include "MetaballsComputeShader.hpp"
#include "MetaballsShader.hpp"

#include "Shader/AlphaTestShader.hpp"

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
            auto light = Engine::getSingleComponent<LightComponent>();
            light->imGuiEditor();
            if (auto spatial = light->getGameObject().getComponentByType<SpatialComponent>()) {
                spatial->imGuiEditor();
            }
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
    config.APP_NAME = "Compute";
    config.APP_RES = "res/";
    config.attachEditor = false;
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    // Create mesh
    {
        auto& go = Engine::createGameObject();
        Engine::addComponent<MetaballsMeshComponent>(&go);
        Engine::addComponent<SpatialComponent>(&go, glm::vec3(0.f, 0.5f, 0.f));
    }

    /* Ground plane */
    Renderable plane(Library::getMesh("quad"), glm::vec3(0.f), glm::vec3(15.f), glm::vec3(-Util::PI() / 2.f, 0.f, 0.f));
    Engine::addComponent<renderable::AlphaTestRenderable>(plane.gameObject);
    Engine::addComponent<DiffuseMapComponent>(plane.gameObject, Library::getTexture("grid.png"));

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<MetaballsSystem>();

    /* Init renderer */
    Renderer::init("shaders/", camera.camera);
    auto& computeShader = Renderer::addComputeShader<MetaballsComputeShader>("metaballs.compute");
    computeShader.mActive = false;
    Renderer::addSceneShader<MetaballsShader>("metaballs.vert", "metaballs.frag");
    Renderer::addSceneShader<AlphaTestShader>();

    Engine::addImGuiFunc("Mesh", []() {
        if (auto mesh = Engine::getComponentTuple<MetaballsMeshComponent, SpatialComponent>()) {
            mesh->get<MetaballsMeshComponent>()->imGuiEditor();
            mesh->get<SpatialComponent>()->imGuiEditor();

            static bool useCompute = false;
            if (ImGui::Checkbox("Use compute", &useCompute)) {
                Renderer::getShader<MetaballsComputeShader>().mActive = useCompute;
                Engine::getSystem<MetaballsSystem>().mActive = !useCompute;
            }
        }

    });

    /* Run */
    Engine::run();
    return 0;
}