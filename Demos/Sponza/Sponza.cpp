#include "Sponza/Sponza.hpp"
#include "Engine/Engine.hpp"

#include "Renderer/Shader/PhongShadowShader.hpp"
#include "Renderer/Shader/ShadowCasterShader.hpp"

#include "Loader/Loader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/RenderableComponent/LineMeshComponent.hpp"
#include "ECS/Component/RenderableComponent/WireframeRenderable.hpp"
#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustaFittingSystem.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
namespace {
    struct Camera {
        CameraComponent* camera;
        Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
            GameObject* gameObject = &ecs.createGameObject("Camera");
            ecs.addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
            camera = &ecs.addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
            ecs.addComponent<CameraControllerComponent>(gameObject, ls, ms);
        }
    };
}

IDemo::Config Sponza::getConfig() const {
    IDemo::Config config;
    config.name = "Sponza";
    config.attachEditor = true;
    return config;
}

void Sponza::init(ECS& ecs) {

    /* Game objects */
    Camera camera(ecs, 45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 50.f);
    ecs.addComponent<MainCameraComponent>(&camera.camera->getGameObject());
    ecs.addComponent<FrustumComponent>(&camera.camera->getGameObject());
    ecs.addComponent<FrustumFitSourceComponent>(&camera.camera->getGameObject());

    {
        auto gameObject = &ecs.createGameObject("Light");
        auto& spat = ecs.addComponent<SpatialComponent>(gameObject, glm::vec3(20.f));
        spat.setLookDir(glm::normalize(glm::vec3(0.43f, -0.464f, -0.776f)));
        ecs.addComponent<LightComponent>(gameObject, glm::vec3(1.f));
            ecs.addComponent<renderable::WireframeRenderable>(gameObject);
        ecs.addComponent<MeshComponent>(gameObject, *Library::getMesh("cube"));
        auto& line = ecs.addComponent<LineMeshComponent>(gameObject, glm::vec3(1, 0, 0));
        line.mUseParentSpatial = true;
        line.addNode({ 0,0,0 });
        line.addNode({ 0,0,1 });
    }
    {
        auto shadowCam = &ecs.createGameObject("shadowcam");
        ecs.addComponentAs<OrthoCameraComponent, CameraComponent>(shadowCam, -1.f, 1000.f, -100.f, 100.f, -100.f, 100.f);
        ecs.addComponent<ShadowCameraComponent>(shadowCam);
        ecs.addComponent<SelectableComponent>(shadowCam);
        ecs.addComponent<FrustumComponent>(shadowCam);
        ecs.addComponent<SpatialComponent>(shadowCam);
        ecs.addComponent<FrustumFitReceiverComponent>(shadowCam);
    }

    auto assets = Loader::loadMultiAsset("sponza.obj");
    for (auto asset : assets) {
        auto gameObject = &ecs.createGameObject();
        ecs.addComponent<MeshComponent>(gameObject, *asset.mesh);
        ecs.addComponent<SpatialComponent>(gameObject, glm::vec3(0.f), glm::vec3(0.1f));
        auto diffuseTex = asset.diffuse_tex ? asset.diffuse_tex : Library::getTexture("black");
        ecs.addComponent<renderable::PhongShadowRenderable>(gameObject, *diffuseTex, asset.material);
        ecs.addComponent<renderable::ShadowCasterRenderable>(gameObject, *diffuseTex);
        // ecs.addComponent<SelectableComponent>(renderable.gameObject);
        ecs.addComponent<BoundingBoxComponent>(gameObject, *asset.mesh);
    }

    /* Systems - order matters! */
    ecs.addSystem<CameraControllerSystem>();
    ecs.addSystem<FrustaFittingSystem>();

    /* Init renderer */
    Renderer::addPreProcessShader<ShadowCasterShader>(4096);
    auto& _s = Renderer::addSceneShader<PhongShadowShader>();
    _s.bias = 0.001f;

;
}