#include "Base/BaseDemo.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/AlphaTestRenderable.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/PhongRenderable.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/AlphaTestShader.hpp"

#include "Renderer/GLObjects/Material.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */

namespace Base {

    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "Base Demo";
        return config;
    }

    void Demo::init(ECS& ecs, Renderer& renderer) {

        /* Camera */
        {
            auto entity = ecs.createEntity();
            ecs.addComponent<TagComponent>(entity, "Camera");
            ecs.addComponent<SpatialComponent>(entity, glm::vec3(0, 0.6f, 5), glm::vec3(1.f));
            ecs.addComponent<PerspectiveCameraComponent>(entity, 1.f, 100.f, 45.f);
            ecs.addComponent<CameraControllerComponent>(entity, 0.4f, 7.f);
            ecs.addComponent<MainCameraComponent>(entity);
        }

        {
            auto entity = ecs.createEntity();
            ecs.addComponent<TagComponent>(entity, "Light");
            ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 2.f, 20.f));
            ecs.addComponent<LightComponent>(entity, glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
        }

        /* Bunny object */
        auto bunny = ecs.createEntity();
        ecs.addComponent<TagComponent>(bunny, "Bunny");
        ecs.addComponent<SpatialComponent>(bunny, glm::vec3(0.f, 1.0f, 0.f));
        ecs.addComponent<RotationComponent>(bunny, glm::vec3(0.f, 1.0f, 0.f));
        ecs.addComponent<MeshComponent>(bunny, Library::loadMesh("bunny.obj", true).mMesh);
        ecs.addComponent<renderable::PhongRenderable>(bunny, Library::getTexture("black"), Material(glm::vec3(0.2f), glm::vec3(1.f, 0.f, 1.f)));
        ecs.addComponent<BoundingBoxComponent>(bunny, Library::loadMesh("bunny.obj"));

        /* Ground plane */
        auto plane = ecs.createEntity();
        ecs.addComponent<TagComponent>(plane, "Grid");
        ecs.addComponent<SpatialComponent>(plane, glm::vec3(0.f), glm::vec3(15.f), glm::vec3(-util::PI / 2.f, 0.f, 0.f));
        ecs.addComponent<MeshComponent>(plane, Library::getMesh("quad").mMesh);
        ecs.addComponent<renderable::AlphaTestRenderable>(plane, Library::loadTexture("grid.png"));

        /* Systems - order matters! */
        ecs.addSystem<CameraControllerSystem>();
        ecs.addSystem<RotationSystem>();

        /* Init renderer */
        renderer.addSceneShader<PhongShader>();
        renderer.addSceneShader<AlphaTestShader>();
    }

    void Demo::update(ECS& ecs) {
        NEO_UNUSED(ecs);
    }

    void Demo::destroy() {
    }

}
