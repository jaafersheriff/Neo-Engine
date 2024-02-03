#include "Sponza/Sponza.hpp"

#include "GBufferComponent.hpp"
#include "GBufferRenderer.hpp"
#include "LightPassRenderer.hpp"
#include "AORenderer.hpp"

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
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/SpatialComponent/SinTranslateComponent.hpp"
#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"
#include "ECS/Component/RenderingComponent/ShadowCasterShaderComponent.hpp"
#include "ECS/Component/RenderingComponent/WireframeShaderComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumCullingSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustaFittingSystem.hpp"
#include "ECS/Systems/TranslationSystems/SinTranslateSystem.hpp"

#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/RenderingSystems/ShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
namespace Sponza {
    namespace {

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

        void _createPointLights(ECS& ecs, const int count) {
            ecs.getView<PointLightComponent>().each([&ecs](auto entity, auto comp) {
                NEO_UNUSED(comp);
                ecs.removeEntity(entity);
            });

            for (int i = 0; i < count; i++) {
                glm::vec3 position(
                    util::genRandom(-150.f, 200.f),
                    util::genRandom(0.f, 85.f),
                    util::genRandom(-100.f, 100.f)
                );
                glm::vec3 scale(util::genRandom(10.f, 40.f));
                const auto entity = ecs.createEntity();
                ecs.addComponent<LightComponent>(entity, util::genRandomVec3(0.3f, 1.f));
                ecs.addComponent<PointLightComponent>(entity);
                ecs.addComponent<SinTranslateComponent>(entity, glm::vec3(0.f, util::genRandom(0.f, 45.f), 0.f), position);
                ecs.addComponent<SpatialComponent>(entity, position, scale);
            }
        }
    }

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
            ecs.addComponent<MainLightComponent>(lightEntity);
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
        ecs.addSystem<SinTranslateSystem>();
        ecs.addSystem<FrustumSystem>();
        ecs.addSystem<FrustaFittingSystem>();
        ecs.addSystem<FrustumCullingSystem>();
    }

    void Demo::imGuiEditor(ECS& ecs) {
        ImGui::Checkbox("Shadows", &mDrawShadows);

        if (ImGui::Checkbox("Deferred Shading", &mDeferredShading)) {
            if (mDeferredShading) {
                _createPointLights(ecs, mPointLightCount);
            }
        }
        if (mDeferredShading) {
            ImGui::SliderFloat("Debug Radius", &mLightDebugRadius, 0.f, 10.f);
            if (ImGui::SliderInt("# Point Lights", &mPointLightCount, 0, 100)) {
                _createPointLights(ecs, mPointLightCount);
            }

            ImGui::Checkbox("AO", &mDrawAO);
            if (mDrawAO) {
                ImGui::SliderFloat("AO Radius", &mAORadius, 0.f, 1.f);
                ImGui::SliderFloat("AO Bias", &mAOBias, 0.f, 1.f);
            }
        }
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
        if (mDrawShadows) {
            drawShadows<OpaqueComponent>(*shadowMap, ecs);
            drawShadows<AlphaTestComponent>(*shadowMap, ecs);
        }

        if (mDeferredShading) {
            _deferredShading(ecs, *sceneTarget, viewport.mSize, mDrawShadows ? shadowMap->mTextures[0] : nullptr);
        }
        else {
            _forwardShading(ecs, *sceneTarget, mDrawShadows ? shadowMap->mTextures[0] : nullptr);
        }

        backbuffer.bind();
        backbuffer.clear(glm::vec4(0,0,0, 1.f), GL_COLOR_BUFFER_BIT);
        drawFXAA(backbuffer, *sceneTarget->mTextures[0]);
       // Don't forget the depth. Because reasons.
       glBlitNamedFramebuffer(sceneTarget->mFBOID, backbuffer.mFBOID,
           0, 0, viewport.mSize.x, viewport.mSize.y,
           0, 0, viewport.mSize.x, viewport.mSize.y,
           GL_DEPTH_BUFFER_BIT,
           GL_NEAREST
       );
    }

    void Demo::_forwardShading(const ECS& ecs, Framebuffer& sceneTarget, Texture* shadowMap) {
        TRACY_GPU();
        const auto&& [cameraEntity, _, __] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();

        sceneTarget.bind();
        sceneTarget.clear(glm::vec4(getConfig().clearColor, 0.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, sceneTarget.mTextures[0]->mWidth, sceneTarget.mTextures[0]->mHeight);
        drawPhong<OpaqueComponent>(ecs, cameraEntity, shadowMap);
        drawPhong<AlphaTestComponent>(ecs, cameraEntity, shadowMap);
    }

    void Demo::_deferredShading(const ECS& ecs, Framebuffer& sceneTarget, glm::uvec2 targetSize, Texture* shadowMap) {
        TRACY_GPU();
        const auto&& [cameraEntity, _, __] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();
        auto& gbuffer = createGBuffer(targetSize);
        gbuffer.bind();
        gbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, targetSize.x, targetSize.y);
        drawGBuffer<OpaqueComponent>(ecs, cameraEntity, {});
        drawGBuffer<AlphaTestComponent>(ecs, cameraEntity, {});

        auto ao = mDrawAO ? drawAO(ecs, cameraEntity, gbuffer, targetSize, mAORadius, mAOBias) : nullptr;

        auto lightResolve = Library::getPooledFramebuffer({ targetSize, {
            TextureFormat{
                TextureTarget::Texture2D,
                GL_RGB16F,
                GL_RGB,
            }
        } }, "LightResolve");
        lightResolve->bind();
        lightResolve->clear(glm::vec4(0.f, 0.f, 0.f, 1.f), GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, targetSize.x, targetSize.y);
        drawPointLights(ecs, gbuffer, cameraEntity, targetSize, mLightDebugRadius);
        drawDirectionalLights(ecs, cameraEntity, gbuffer, shadowMap);

        // I'm lazy so I'm just going to do the final combine here
        {
            TRACY_GPUN("Final Combine");
            sceneTarget.bind();
            sceneTarget.clear(glm::vec4(0.f, 0.f, 0.f, 0.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, sceneTarget.mTextures[0]->mWidth, sceneTarget.mTextures[0]->mHeight);
            auto* combineShader = Library::createSourceShader("FinalCombine", SourceShader::ConstructionArgs{
                { ShaderStage::VERTEX, "quad.vert"},
                { ShaderStage::FRAGMENT, "sponza/combine.frag" }
                });
            ShaderDefines defines;
            MakeDefine(DRAW_AO);
            if (mDrawAO) {
                defines.set(DRAW_AO);
            }
            auto& resolvedShader = combineShader->getResolvedInstance(defines);
            resolvedShader.bind();

            resolvedShader.bindTexture("lightOutput", *lightResolve->mTextures[0]);
            if (mDrawAO) {
                resolvedShader.bindTexture("aoOutput", *ao->mTextures[0]);
            }

            Library::getMesh("quad").mMesh->draw();

            // Don't forget the depth. Because reasons.
            glBlitNamedFramebuffer(gbuffer.mFBOID, sceneTarget.mFBOID,
                0, 0, targetSize.x, targetSize.y,
                0, 0, targetSize.x, targetSize.y,
                GL_DEPTH_BUFFER_BIT,
                GL_NEAREST
            );
        }
    }
}