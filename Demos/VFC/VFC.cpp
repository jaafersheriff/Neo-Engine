#include "VFC.hpp"
#include "Engine/Engine.hpp"

#include "Renderer/Shader/GammaCorrectShader.hpp"
#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/AlphaTestShader.hpp"
#include "Renderer/Shader/LineShader.hpp"

#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/WireframeRenderable.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumToLineSystem.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
namespace {
    struct Camera {
        GameObject* gameObject;
        CameraComponent* camera;
        Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos) {
            gameObject = &ecs.createGameObject();
            ecs.addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
            camera = &ecs.addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
        }
    };

    struct Light {
        Light(ECS& ecs, glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
            auto& gameObject = ecs.createGameObject();
            ecs.addComponent<SpatialComponent>(&gameObject, pos);
            ecs.addComponent<LightComponent>(&gameObject, col, att);

            Engine::addImGuiFunc("Light", [](ECS& ecs_) {
                ecs_.getSingleComponent<LightComponent>()->imGuiEditor();
                });
        }
    };

    struct Renderable {
        GameObject* gameObject;

        Renderable(ECS& ecs, Mesh* mesh, glm::vec3 position = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f), glm::vec3 rotation = glm::vec3(0.f)) {
            gameObject = &ecs.createGameObject();
            ecs.addComponent<MeshComponent>(gameObject, *mesh);
            ecs.addComponent<SpatialComponent>(gameObject, position, scale, rotation);
        }
    };

    void generateObjects(ECS& ecs, int amount) {
        for (auto& comp : ecs.getComponents<BoundingBoxComponent>()) {
            ecs.removeGameObject(comp->getGameObject());
        }
        for (auto& comp : ecs.getComponents<renderable::WireframeRenderable>()) {
            ecs.removeGameObject(comp->getGameObject());
        }

        for (int i = 0; i < amount; i++) {
            glm::vec3 position(util::genRandom(-15.f, 15.f), 0.f, util::genRandom(-15.f, 15.f));
            glm::vec3 size = glm::vec3(util::genRandom(0.5f, 2.f), util::genRandom(0.5f, 2.f), util::genRandom(0.5f, 2.f));
            const auto mesh = Library::getMesh("sphere");

            Renderable renderable(ecs, mesh, position, size);
            Material material;
            material.mAmbient = glm::vec3(0.2f);
            material.mDiffuse = (glm::normalize(position) + 1.f) / 2.f;
            ecs.addComponent<renderable::PhongRenderable>(renderable.gameObject, *Library::getTexture("black"), material);
            auto boundingBox = &ecs.addComponent<BoundingBoxComponent>(renderable.gameObject, *mesh);
            NEO_UNUSED(boundingBox);
        }
    }

}

IDemo::Config VFC::getConfig() const {
    IDemo::Config config;
    config.name = "VFC";
    return config;
}

void VFC::init(ECS& ecs) {

    /* Game objects */
    Camera camera(ecs, 45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5));
    ecs.addComponent<MainCameraComponent>(&camera.camera->getGameObject());
    ecs.addComponent<CameraControllerComponent>(camera.gameObject, 0.4f, 7.f);
    ecs.addComponent<FrustumComponent>(camera.gameObject);

    Light(ecs, glm::vec3(-100.f, 100.f, 100.f), glm::vec3(1.f), glm::vec3(0.f, 0.015f, 0.f));

    // random objects
    generateObjects(ecs, 10);

    /* Ground plane */
    Renderable plane(ecs, Library::getMesh("quad"), glm::vec3(0.f), glm::vec3(30.f), glm::vec3(-util::PI / 2.f, 0.f, 0.f));
    ecs.addComponent<renderable::AlphaTestRenderable>(plane.gameObject, *Library::loadTexture("grid.png"));

    /* Systems - order matters! */
    ecs.addSystem<CameraControllerSystem>();
    ecs.addSystem<FrustumSystem>();
    ecs.addSystem<FrustumToLineSystem>();

    /* Init renderer */
    Renderer::addSceneShader<AlphaTestShader>();
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<LineShader>();
    Renderer::addPostProcessShader<GammaCorrectShader>();

    /* Attach ImGui panes */
    Engine::addImGuiFunc("VFC", [](ECS& ecs_) {
        static int slider = 10;
        ImGui::SliderInt("Num objects", &slider, 1, 10000);
        if (ImGui::Button("Regenerate")) {
            generateObjects(ecs_, slider);
        }
    });
}
