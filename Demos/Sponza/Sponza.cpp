#include "Sponza/Sponza.hpp"

#include "GBufferComponent.hpp"
#include "GBufferRenderer.hpp"

#include "Engine/Engine.hpp"
#include "Loader/Loader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/ShadowCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"
#include "ECS/Component/RenderingComponent/ShadowCasterShaderComponent.hpp"
#include "ECS/Component/RenderingComponent/WireframeShaderComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumCullingSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustaFittingSystem.hpp"

#include "Renderer/RenderingSystems/Blitter.hpp"
#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/RenderingSystems/ShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"

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
            ecs.addComponent<LightComponent>(lightEntity, glm::vec3(1.f));
            ecs.addComponent<DirectionalLightComponent>(lightEntity);
            ecs.addComponent<WireframeShaderComponent>(lightEntity);
            ecs.addComponent<MeshComponent>(lightEntity, Library::getMesh("cube").mMesh);
            auto line = ecs.addComponent<LineMeshComponent>(lightEntity, glm::vec3(1, 0, 0));
            line->mUseParentSpatial = true;
            line->addNode({ 0,0,0 });
            line->addNode({ 0,0,1 });
        }
        {
            auto shadowCam = ecs.createEntity();
            ecs.addComponent<TagComponent>(shadowCam, "Shadow Camera");
            ecs.addComponent<OrthoCameraComponent>(shadowCam, -1.f, 1000.f, -100.f, 100.f, -100.f, 100.f);
            ecs.addComponent<ShadowCameraComponent>(shadowCam);
            ecs.addComponent<FrustumComponent>(shadowCam);
            ecs.addComponent<SpatialComponent>(shadowCam);
            ecs.addComponent<FrustumFitReceiverComponent>(shadowCam, 1.f);
        }

        auto assets = Loader::loadMultiAsset("sponza.obj");
        for (auto& asset : assets) {
            auto entity = ecs.createEntity();
            ecs.addComponent<MeshComponent>(entity, asset.meshData.mMesh);
            ecs.addComponent<SpatialComponent>(entity, asset.meshData.mBasePosition * 0.1f, asset.meshData.mBaseScale * 0.1f);
            ecs.addComponent<ShadowCasterShaderComponent>(entity);
            ecs.addComponent<BoundingBoxComponent>(entity, asset.meshData);
            ecs.addComponent<PhongShaderComponent>(entity);
            ecs.addComponent<GBufferShaderComponent>(entity);
            asset.material.mAmbient = glm::vec3(0.3f);
            auto material = ecs.addComponent<MaterialComponent>(entity, asset.material);
            if (material->mAlphaMap) {
                ecs.addComponent<AlphaTestComponent>(entity);
            }
            else {
                ecs.addComponent<OpaqueComponent>(entity);
            }
        }


        /* Systems - order matters! */
        auto& camSys = ecs.addSystem<CameraControllerSystem>();
        camSys.mSuperSpeed = 10.f;
        ecs.addSystem<FrustumSystem>();
        ecs.addSystem<FrustaFittingSystem>();
        ecs.addSystem<FrustumCullingSystem>();
    }

    void Demo::imGuiEditor(ECS& ecs) {
        NEO_UNUSED(ecs);
 
        ImGui::Checkbox("Deferred Shading", &mDeferredShading);
    }

    void Demo::render(const ECS& ecs, Framebuffer& backbuffer) {
        auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
        auto sceneTarget = Library::getPooledFramebuffer({ viewport.mSize, {
            TextureFormat{
                TextureTarget::Texture2D,
                GL_RGB16,
                GL_RGB,
            },
            TextureFormat{
                TextureTarget::Texture2D,
                GL_DEPTH_COMPONENT16,
                GL_DEPTH_COMPONENT,
            }
        } }, "Scene target");

        auto shadowMap = Library::getPooledFramebuffer({ glm::uvec2(4096, 4096), { 
            TextureFormat{
                TextureTarget::Texture2D,
                GL_DEPTH_COMPONENT16,
                GL_DEPTH_COMPONENT
            } 
        } }, "Shadow map");
        shadowMap->bind();
        shadowMap->clear(glm::uvec4(0.f, 0.f, 0.f, 0.f), GL_DEPTH_BUFFER_BIT);
        drawShadows<OpaqueComponent>(*shadowMap, ecs);
        drawShadows<AlphaTestComponent>(*shadowMap, ecs);

        if (mDeferredShading) {
            _deferredShading(ecs, *sceneTarget, viewport.mSize);
        }
        else {
            _forwardShading(ecs, *sceneTarget, shadowMap->mTextures[0]);
        }

        backbuffer.bind();
        backbuffer.clear(glm::vec4(getConfig().clearColor, 1.f), GL_COLOR_BUFFER_BIT);
        drawFXAA(backbuffer, *sceneTarget->mTextures[0]);
    }

    void Demo::_forwardShading(const ECS& ecs, Framebuffer& sceneTarget, Texture* shadowMap) {
        const auto&& [cameraEntity, _, __] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();

        sceneTarget.bind();
        sceneTarget.clear(glm::vec4(getConfig().clearColor, 0.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, sceneTarget.mTextures[0]->mWidth, sceneTarget.mTextures[0]->mHeight);
        drawPhong<OpaqueComponent>(ecs, cameraEntity, shadowMap);
        drawPhong<AlphaTestComponent>(ecs, cameraEntity, shadowMap);
    }

    void Demo::_deferredShading([[maybe_unused]] const ECS& ecs, [[maybe_unused]] Framebuffer& sceneTarget, glm::uvec2 targetSize) {
        const auto&& [cameraEntity, _, __] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();
        auto& gbuffer = createGBuffer(targetSize);
        gbuffer.bind();
        gbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, targetSize.x, targetSize.y);
        drawGBuffer<OpaqueComponent>(ecs, cameraEntity, {});
        drawGBuffer<AlphaTestComponent>(ecs, cameraEntity, {});

        blit(sceneTarget, *gbuffer.mTextures[0], targetSize);
    }
}