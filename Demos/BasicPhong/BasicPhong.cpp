#include "BasicPhong/BasicPhong.hpp"
#include "Engine/Engine.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/WireframeShader.hpp"

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
#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"

using namespace neo;

/* Game object definitions */
namespace BasicPhong {
    struct Camera {
        ECS::Entity mEntity;
        Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
            mEntity = ecs.createEntity();
            ecs.addComponent<TagComponent>(mEntity, "Camera");
            ecs.addComponent<SpatialComponent>(mEntity, pos, glm::vec3(1.f), glm::vec3(3.f));
            ecs.addComponent<PerspectiveCameraComponent>(mEntity, near, far, fov);
            ecs.addComponent<CameraControllerComponent>(mEntity, ls, ms);
        }
    };

    struct Light {
        Light(ECS& ecs, glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
            auto entity = ecs.createEntity();
            ecs.addComponent<TagComponent>(entity, "Light");
            ecs.addComponent<SpatialComponent>(entity, pos);
            ecs.addComponent<LightComponent>(entity, col, att);
            ecs.addComponent<MeshComponent>(entity, *Library::getMesh("sphere").mMesh);
            ecs.addComponent<BoundingBoxComponent>(entity, Library::getMesh("sphere"));
            ecs.addComponent<renderable::WireframeRenderable>(entity);
            ecs.addComponent<SelectableComponent>(entity);
        }
    };

    struct Renderable {
        Renderable(ECS& ecs, Mesh* mesh, Texture* tex, glm::vec3 p, float s = 1.f, glm::mat3 o = glm::mat3(1.f)) {
            auto entity = ecs.createEntity();
            ecs.addComponent<SpatialComponent>(entity, p, glm::vec3(s), o);
            ecs.addComponent<MeshComponent>(entity, *mesh);
            Material material;
            material.mAmbient = glm::vec3(0.1f);
            material.mDiffuse = glm::vec3(0.f);
            material.mSpecular = glm::vec3(1.f);
            material.mShininess = 50.f;
            ecs.addComponent<renderable::PhongRenderable>(entity, *tex, material);
        }
    };

    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "Basic Phong";
        config.clearColor = { 0.1f, 0.1f, 0.1f };
        return config;
    }

    void Demo::init(ECS& ecs, Renderer& renderer) {

        /* Game objects */
        Camera camera(ecs, 45.f, 0.01f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
        ecs.addComponent<MainCameraComponent>(camera.mEntity);

        Light(ecs, glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

        Library::loadMesh("mr_krab.obj", true);
        Library::loadTexture("mr_krab.png");
        std::vector<Renderable*> renderables;
        for (int x = -2; x < 3; x++) {
            for (int z = 0; z < 10; z++) {
                renderables.push_back(
                    new Renderable(
                        ecs,
                        Library::getMesh("mr_krab.obj").mMesh,
                        Library::getTexture("mr_krab.png"),
                        glm::vec3(x * 2, 0, z * 2))
                );
            }
        }

        /* Systems - order matters! */
        ecs.addSystem<CameraControllerSystem>();

        /* Init renderer */
        renderer.addSceneShader<PhongShader>();
        renderer.addSceneShader<WireframeShader>();

        /* Attach ImGui panes */
    }

    void Demo::update(ECS& ecs) {
        NEO_UNUSED(ecs);
    }

    void Demo::destroy() {
    }

}
