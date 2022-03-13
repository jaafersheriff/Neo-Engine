#include "Compute.hpp"
#include "Engine/Engine.hpp"

#include "ParticleMeshComponent.hpp"
#include "ParticlesComputeShader.hpp"
#include "ParticleVisShader.hpp"

#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"

using namespace neo;

/* Game object definitions */
namespace Compute {
    struct Camera {
        ECS::Entity mEntity;
        Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
            mEntity = ecs.createEntity();
            ecs.addComponent<SpatialComponent>(mEntity, pos, glm::vec3(1.f));
            ecs.addComponent<PerspectiveCameraComponent>(mEntity, near, far, fov);
            ecs.addComponent<CameraControllerComponent>(mEntity, ls, ms);
        }
    };

    struct Light {
        Light(ECS& ecs, glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
            auto entity = ecs.createEntity();
            ecs.addComponent<SpatialComponent>(entity, pos);
            ecs.addComponent<LightComponent>(entity, col, att);
        }
    };

    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "Compute";
        config.clearColor = { 0.f, 0.f, 0.f };
        return config;
    }

    void Demo::init(ECS& ecs, Renderer& renderer) {

        /* Game objects */
        Camera camera(ecs, 45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
        ecs.addComponent<MainCameraComponent>(camera.mEntity);

        Light(ecs, glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

        // Create mesh
        {
            auto entity = ecs.createEntity();
            ecs.addComponent<ParticleMeshComponent>(entity);
            ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 0.0f, 0.f));
        }

        /* Systems - order matters! */
        ecs.addSystem<CameraControllerSystem>();

        /* Init renderer */
        renderer.addComputeShader<ParticlesComputeShader>("compute/particles.compute");
        renderer.addSceneShader<ParticleVisShader>("compute/particles.vert", "compute/particles.frag", "compute/particles.geom");
    }

    void Demo::imGuiEditor(ECS& ecs) {
        if (auto particles = ecs.getSingleView<ParticleMeshComponent, SpatialComponent>()) {
            auto&& [_, mesh, spatial] = *particles;
            mesh.imGuiEditor();
            spatial.imGuiEditor();
        }
    }
}