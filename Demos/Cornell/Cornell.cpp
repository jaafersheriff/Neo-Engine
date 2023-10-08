#include "Cornell/Cornell.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/PhongRenderable.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/FXAAShader.hpp"

#include "Renderer/GLObjects/Material.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */

namespace Cornell {
    namespace {
        inline void insertObject(ECS& ecs, std::string name, Mesh* mesh, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, glm::vec3 color) {
            auto entity = ecs.createEntity();
            ecs.addComponent<TagComponent>(entity, name);
            ecs.addComponent<MeshComponent>(entity, mesh);
            ecs.addComponent<SpatialComponent>(entity, position, scale, rotation);

            Material material;
            material.mAmbient = glm::vec3(0.2f);
            material.mDiffuse = color;
            material.mSpecular = glm::vec3(1.f);
            material.mShininess = 1.f;
            ecs.addComponent<renderable::PhongRenderable>(entity, Library::getTexture("white"), material);
        }
    }

    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "Cornell Demo";
        config.clearColor = glm::vec3(0.f);
        return config;
    }

    void Demo::init(ECS& ecs, Renderer& renderer) {

        /* Camera */
        {
            auto entity = ecs.createEntity();
            ecs.addComponent<TagComponent>(entity, "Camera");
            ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 0.5f, 2.25f), glm::vec3(1.f));
            ecs.addComponent<PerspectiveCameraComponent>(entity, 0.1f, 100.f, 45.f);
            ecs.addComponent<CameraControllerComponent>(entity, 0.4f, 7.f);
            ecs.addComponent<MainCameraComponent>(entity);
        }

        {
            auto entity = ecs.createEntity();
            ecs.addComponent<TagComponent>(entity, "Light");
            ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 1.f - util::EP * 4, 0.5f), glm::vec3(0.25f), glm::vec3(glm::radians(90.f), 0.f, 0.f));
            ecs.addComponent<LightComponent>(entity, glm::vec3(1.f), glm::vec3(3.0, 1.0, 5.0f));
            ecs.addComponent<MeshComponent>(entity, Library::getMesh("quad").mMesh);
            ecs.addComponent<renderable::PhongRenderable>(entity, Library::getTexture("white"), Material{});
        }

        /* Bunny object */
        insertObject(ecs, "backwall",  Library::getMesh("quad").mMesh, glm::vec3(0.f, 0.5f, 0.f), glm::vec3(1.f), glm::vec3(0.f), glm::vec3(1.f));
        insertObject(ecs, "leftwall",  Library::getMesh("quad").mMesh, glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.f), glm::vec3(0.f, glm::radians(90.f), 0.f), glm::vec3(1.f, 0.f, 0.f));
        insertObject(ecs, "rightwall", Library::getMesh("quad").mMesh, glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.f), glm::vec3(0.f, glm::radians(-90.f), 0.f), glm::vec3(0.f, 1.f, 0.f));
        insertObject(ecs, "floor", Library::getMesh("quad").mMesh, glm::vec3(0.f, 0.f, 0.5f), glm::vec3(1.f), glm::vec3(glm::radians(-90.f), 0.f, 0.f), glm::vec3(1.f));
        insertObject(ecs, "ceiling", Library::getMesh("quad").mMesh, glm::vec3(0.f, 1.0f, 0.5f), glm::vec3(1.f), glm::vec3(glm::radians(90.f), 0.f, 0.f), glm::vec3(1.f));
        insertObject(ecs, "box1", Library::getMesh("cube").mMesh, glm::vec3(-0.2f, 0.35f, 0.4f), glm::vec3(0.25f, 0.7f, 0.25f), glm::vec3(0.f, glm::radians(33.f), 0.f), glm::vec3(1.f));
        insertObject(ecs, "box2", Library::getMesh("cube").mMesh, glm::vec3(0.2f, 0.15f, 0.6f), glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0.f, glm::radians(-17.f), 0.f), glm::vec3(1.f));

        /* Systems - order matters! */
        ecs.addSystem<CameraControllerSystem>();

        /* Init renderer */
        renderer.addSceneShader<PhongShader>();
        renderer.addPostProcessShader<FXAAShader>();
    }

    void Demo::update(ECS& ecs) {
        NEO_UNUSED(ecs);
    }

    void Demo::destroy() {
    }

}
