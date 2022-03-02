#include "Selecting/Selecting.hpp"
#include "Engine/Engine.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/AlphaTestShader.hpp"
#include "Renderer/Shader/LineShader.hpp"
#include "Renderer/Shader/SelectableShader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/SelectingSystems/MouseRaySystem.hpp"
#include "ECS/Systems/SelectingSystems/SelectingSystem.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

namespace Selecting {

    /* Game object definitions */
    namespace {
        struct Camera {
            CameraComponent* camera;
            Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
                GameObject* gameObject = &ecs.createGameObject();
                ecs.addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
                camera = &ecs.addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
                ecs.addComponent<CameraControllerComponent>(gameObject, ls, ms);
            }
        };

        struct Light {
            Light(ECS& ecs, glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
                auto& gameObject = ecs.createGameObject("Light");
                ecs.addComponent<SpatialComponent>(&gameObject, pos);
                ecs.addComponent<LightComponent>(&gameObject, col, att);
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
    }

    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "Selecting";
        config.attachEditor = false;
        return config;
    }

    void Demo::init(ECS& ecs) {

        /* Game objects */
        Camera camera(ecs, 45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
        ecs.addComponent<MainCameraComponent>(&camera.camera->getGameObject());

        Light(ecs, glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

        /* Cube object */
        for (int i = 0; i < 30; i++) {
            Renderable r(ecs, Library::getMesh("sphere").mMesh, glm::vec3(util::genRandom(-7.5, 7.5), 0.f, util::genRandom(-7.5, 7.5)));
            Material material;
            material.mAmbient = glm::vec3(0.2f);
            material.mDiffuse = glm::vec3(1.f, 1.f, 1.f);
            material.mShininess = 20.f;
            ecs.addComponent<renderable::PhongRenderable>(r.gameObject, *Library::getTexture("black"), material);
            ecs.addComponent<BoundingBoxComponent>(r.gameObject, Library::getMesh("sphere"));
            ecs.addComponent<SelectableComponent>(r.gameObject);
        }

        /* Ground plane */
        {
            Renderable plane(ecs, Library::getMesh("quad").mMesh, glm::vec3(0.f), glm::vec3(15.f), glm::vec3(-util::PI / 2.f, 0.f, 0.f));
            ecs.addComponent<renderable::AlphaTestRenderable>(plane.gameObject, *Library::loadTexture("grid.png"));
        }

        /* Systems - order matters! */
        ecs.addSystem<CameraControllerSystem>();
        ecs.addSystem<MouseRaySystem>(true);
        ecs.addSystem<SelectingSystem>(
            "Selecter System",
            // Reset operation for unselected components
            [](ECS& ecs_, SelectedComponent* reset) {
                NEO_UNUSED(ecs_);
                if (auto renderable = reset->getGameObject().getComponentByType<renderable::PhongRenderable>()) {
                    renderable->mMaterial.mDiffuse = glm::vec3(1.f);
                }
            },
            // Operate on selected components
                [](ECS& ecs_, SelectableComponent* selected) {
                NEO_UNUSED(ecs_);
                if (auto renderable = selected->getGameObject().getComponentByType<renderable::PhongRenderable>()) {
                    renderable->mMaterial.mDiffuse = glm::vec3(1.f, 0.f, 0.f);
                }
            },
                // imgui editor
                [](ECS& ecs_, SelectedComponent* edit) {
                NEO_UNUSED(ecs_);
                edit->getGameObject().getComponentByType<SpatialComponent>()->imGuiEditor();
            }
            );

        /* Init renderer */
        Renderer::addPreProcessShader<SelectableShader>();
        Renderer::addSceneShader<PhongShader>();
        Renderer::addSceneShader<AlphaTestShader>();
        Renderer::addSceneShader<LineShader>();

    }
}
