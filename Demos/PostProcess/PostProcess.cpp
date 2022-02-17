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
        GameObject* gameObject;

        Light(ECS& ecs, glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
            gameObject = &ecs.createGameObject();
            ecs.addComponent<SpatialComponent>(gameObject, pos);
            ecs.addComponent<LightComponent>(gameObject, col, att);
        }
    };

    struct Renderable {
        GameObject* gameObject;

        Renderable(ECS& ecs, Mesh* mesh, Texture* texture, float amb, glm::vec3 diffuse, glm::vec3 spec) {
            gameObject = &ecs.createGameObject();
            ecs.addComponent<SpatialComponent>(gameObject, glm::vec3(0.f), glm::vec3(1.f));
            ecs.addComponent<MeshComponent>(gameObject, *mesh);
            Material material;
            material.mAmbient = glm::vec3(amb);
            material.mDiffuse = diffuse;
            material.mSpecular = spec;
            ecs.addComponent<renderable::PhongRenderable>(gameObject, *texture, material);
        }
    };
}

IDemo::Config PostProcess::getConfig() const {
    IDemo::Config config;
    config.name = "VFC";
    return config;
}

void PostProcess::init(ECS& ecs) {

    /* Game objects */
    Camera camera(ecs, 45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    ecs.addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(ecs, glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    Renderable r(ecs, Library::loadMesh("mr_krab.obj"), Library::loadTexture("mr_krab.png"), 0.2f, glm::vec3(1.f, 0.f, 1.f), glm::vec3(1.f));

    /* Systems - order matters! */
    ecs.addSystem<CameraControllerSystem>();

    /* Init renderer */
    Renderer::addSceneShader<PhongShader>();
    Renderer::addPostProcessShader<PostProcessShader>("DepthShader", std::string("depth.frag"));
    Renderer::addPostProcessShader<PostProcessShader>("BlueShader", std::string("blue.frag"));
    Renderer::addPostProcessShader<PostProcessShader>("InvertShader", std::string("invert.frag"));

    /* Attach ImGui panes */
}
