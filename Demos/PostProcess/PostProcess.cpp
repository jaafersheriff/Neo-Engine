#include "PostProcess/PostProcess.hpp"
#include "Engine/Engine.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/PostProcessShader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
namespace PostProcess {

    struct Camera {
        Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
            auto entity = ecs.createEntity();
            ecs.addComponent<SpatialComponent>(entity, pos, glm::vec3(1.f));
            ecs.addComponent<PerspectiveCameraComponent>(entity, near, far, fov);
            ecs.addComponent<CameraControllerComponent>(entity, ls, ms);
            ecs.addComponent<MainCameraComponent>(entity);
        }
    };

    struct Light {
        Light(ECS& ecs, glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
            auto entity = ecs.createEntity();
            ecs.addComponent<SpatialComponent>(entity, pos);
            ecs.addComponent<LightComponent>(entity, col, att);
        }
    };

    struct Renderable {
        Renderable(ECS& ecs, Mesh* mesh, Texture* texture, float amb, glm::vec3 diffuse, glm::vec3 spec) {
            auto entity = ecs.createEntity();
            ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f), glm::vec3(1.f));
            ecs.addComponent<MeshComponent>(entity, *mesh);
            Material material;
            material.mAmbient = glm::vec3(amb);
            material.mDiffuse = diffuse;
            material.mSpecular = spec;
            ecs.addComponent<renderable::PhongRenderable>(entity, *texture, material);
        }
    };

    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "Post Process";
        config.shaderDir = "shaders/postprocess/";
        return config;
    }

    void Demo::init(ECS& ecs, Renderer& renderer) {

        /* Game objects */
        Camera camera(ecs, 45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);

        Light(ecs, glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
        Renderable r(ecs, Library::loadMesh("mr_krab.obj").mMesh, Library::loadTexture("mr_krab.png"), 0.2f, glm::vec3(1.f, 0.f, 1.f), glm::vec3(1.f));

        /* Systems - order matters! */
        ecs.addSystem<CameraControllerSystem>();

        /* Init renderer */
        renderer.addSceneShader<PhongShader>();
        renderer.addPostProcessShader<PostProcessShader>("DepthShader", std::string("depth.frag"));
        renderer.addPostProcessShader<PostProcessShader>("BlueShader", std::string("blue.frag"));
        renderer.addPostProcessShader<PostProcessShader>("InvertShader", std::string("invert.frag"));

        /* Attach ImGui panes */
    }
}
