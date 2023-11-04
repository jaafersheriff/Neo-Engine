#include "Sponza/Sponza.hpp"
#include "Engine/Engine.hpp"

#include "Loader/Loader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/RenderableComponent/LineMeshComponent.hpp"
// #include "ECS/Component/RenderableComponent/WireframeRenderable.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/NewRenderingComponents/AlphaTestComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumCullingSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustaFittingSystem.hpp"

#include "Renderer/RenderingSystems/PhongRenderer.hpp"

#include "Renderer/Shader/PhongShadowShader.hpp"
#include "Renderer/Shader/ShadowCasterShader.hpp"
#include "Renderer/Shader/FXAAShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
namespace Sponza {
    struct Camera {
        ECS::Entity mEntity;
        Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
            mEntity = ecs.createEntity();
            ecs.addComponent<TagComponent>(mEntity, "Camera");
            ecs.addComponent<SpatialComponent>(mEntity, pos, glm::vec3(1.f));
            ecs.addComponent<PerspectiveCameraComponent>(mEntity, near, far, fov);
            ecs.addComponent<CameraControllerComponent>(mEntity, ls, ms);
        }
    };


    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "Sponza";
        return config;
    }

    void Demo::init(ECS& ecs) {

        /* Game objects */
        Camera camera(ecs, 45.f, 1.f, 300.f, glm::vec3(0, 0.6f, 5), 0.4f, 150.f);
        ecs.addComponent<MainCameraComponent>(camera.mEntity);
        ecs.addComponent<FrustumComponent>(camera.mEntity);
        ecs.addComponent<FrustumFitSourceComponent>(camera.mEntity);

        {
            auto lightEntity = ecs.createEntity();
            ecs.addComponent<TagComponent>(lightEntity, "Light");
            auto spat = ecs.addComponent<SpatialComponent>(lightEntity, glm::vec3(75.f, 200.f, 20.f));
            spat->setLookDir(glm::normalize(glm::vec3(-0.28f, -0.96f, -0.06f)));
            ecs.addComponent<LightComponent>(lightEntity, glm::vec3(1.f), glm::vec3(0.9f, 0.7f, 0.f));
            // ecs.addComponent<renderable::WireframeRenderable>(lightEntity);
            ecs.addComponent<MeshComponent>(lightEntity, Library::getMesh("cube").mMesh);
            auto line = ecs.addComponent<LineMeshComponent>(lightEntity, glm::vec3(1, 0, 0));
            line->mUseParentSpatial = true;
            line->addNode({ 0,0,0 });
            line->addNode({ 0,0,1 });
        }
        {
            // auto shadowCam = ecs.createEntity();
            // ecs.addComponent<TagComponent>(shadowCam, "Shadow Camera");
            // ecs.addComponent<OrthoCameraComponent>(shadowCam, -1.f, 1000.f, -100.f, 100.f, -100.f, 100.f);
            // ecs.addComponent<ShadowCameraComponent>(shadowCam);
            // ecs.addComponent<FrustumComponent>(shadowCam);
            // ecs.addComponent<SpatialComponent>(shadowCam);
            // ecs.addComponent<FrustumFitReceiverComponent>(shadowCam, 1.f);
        }

        auto assets = Loader::loadMultiAsset("sponza.obj");
        for (auto& asset : assets) {
            auto entity = ecs.createEntity();
            ecs.addComponent<MeshComponent>(entity, asset.meshData.mMesh);
            ecs.addComponent<SpatialComponent>(entity, asset.meshData.mBasePosition * 0.1f, asset.meshData.mBaseScale * 0.1f);
            auto diffuseTex = asset.diffuse_tex ? asset.diffuse_tex : Library::getTexture("black");
            asset.material.mAmbient = glm::vec3(0.2f);
            // ecs.addComponent<renderable::ShadowCasterRenderable>(entity, diffuseTex);
            ecs.addComponent<BoundingBoxComponent>(entity, asset.meshData);
            // ecs.addComponent<renderable::PhongShadowRenderable>(entity, diffuseTex, asset.material);
            ecs.addComponent<AlphaTestComponent>(entity);
            ecs.addComponent<PhongShaderComponent>(entity);
            auto material = ecs.addComponent<MaterialComponent>(entity, asset.material);
            material->mDiffuseMap = diffuseTex;
        }


        /* Systems - order matters! */
        auto& camSys = ecs.addSystem<CameraControllerSystem>();
        camSys.mSuperSpeed = 10.f;
        ecs.addSystem<FrustumSystem>();
        ecs.addSystem<FrustaFittingSystem>();
        ecs.addSystem<FrustumCullingSystem>();

        /* Init renderer */
        // renderer.addPreProcessShader<ShadowCasterShader>(4096);
        // auto& _s = renderer.addSceneShader<PhongShadowShader>();
        // _s.bias = 0.001f;
        // renderer.addPostProcessShader<FXAAShader>();
    }

    void Demo::render(const ECS& ecs) {
        const auto&& [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();
        const auto&& [__, light, lightSpatial] = *ecs.getSingleView<LightComponent, SpatialComponent>();

        glEnable(GL_BLEND);
        drawPhong<AlphaTestComponent>(ecs, cameraEntity, light, lightSpatial);

    }
}