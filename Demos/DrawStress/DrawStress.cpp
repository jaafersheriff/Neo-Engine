#include "DrawStress.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/NewRenderingComponents/AlphaTestComponent.hpp"
#include "ECS/Component/NewRenderingComponents/OpaqueComponent.hpp"
#include "ECS/Component/RenderableComponent/AlphaTestRenderable.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/PhongRenderable.hpp"
#include "ECS/Component/RenderableComponent/MaterialComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"
#include "ECS/Component/NewRenderingComponents/PhongShaderComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"

#include "Renderer/GLObjects/Material.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */

namespace DrawStress {

    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "DrawStress";
        return config;
    }

    void Demo::init(ECS& ecs) {

        /* Camera */
        {
            auto entity = ecs.createEntity();
            ecs.addComponent<TagComponent>(entity, "Camera");
            ecs.addComponent<SpatialComponent>(entity, glm::vec3(0, 0.6f, 5), glm::vec3(1.f));
            ecs.addComponent<PerspectiveCameraComponent>(entity, 1.f, 1000.f, 45.f);
            ecs.addComponent<CameraControllerComponent>(entity, 0.4f, 7.f);
            ecs.addComponent<MainCameraComponent>(entity);
            ecs.addComponent<FrustumComponent>(entity);
        }

        {
            auto entity = ecs.createEntity();
            ecs.addComponent<TagComponent>(entity, "Light");
            ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 2.f, 20.f));
            ecs.addComponent<LightComponent>(entity, glm::vec3(1.f), glm::vec3(0.1, 0.05, 0.003f));
        }

        /* Bunny object */
        for(int i = 0; i < 6000; i++) {
            auto cube = ecs.createEntity();
            ecs.addComponent<SpatialComponent>(cube, glm::vec3(util::genRandom(-50.f, 50.f), util::genRandom(-10.f, 10.f), util::genRandom(-50.f, 50.f)), glm::vec3(util::genRandom(0.5f, 1.5f)), util::genRandomVec3(-util::PI, util::PI));
            ecs.addComponent<MeshComponent>(cube, Library::loadMesh("cube", true).mMesh);
            ecs.addComponent<BoundingBoxComponent>(cube, Library::loadMesh("cube"));
            ecs.addComponent<PhongShaderComponent>(cube);
            ecs.addComponent<OpaqueComponent>(cube);
            auto material = ecs.addComponent<MaterialComponent>(cube);
            material->mAmbient = glm::vec3(0.2f);
            material->mDiffuse = util::genRandomVec3();
        }

        /* Systems - order matters! */
        ecs.addSystem<CameraControllerSystem>();
        ecs.addSystem<FrustumSystem>();
        ecs.addSystem<FrustumCullingSystem>();
    }

    void Demo::update(ECS& ecs) {
        NEO_UNUSED(ecs);
    }

    void Demo::render(const ECS& ecs, Framebuffer& backbuffer) {
        const auto&& [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();
        const auto light = *ecs.getSingleView<LightComponent, SpatialComponent>();

        auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
        auto sceneTarget = Library::createTransientFBO(viewport.mSize, {
            TextureFormat{
                GL_RGB8,
                GL_RGB,
            },
            TextureFormat{
                GL_R16,
                GL_DEPTH_COMPONENT,
            }
        });

        glm::vec3 clearColor = getConfig().clearColor;

        sceneTarget->bind();
        sceneTarget->clear(glm::vec4(clearColor, 1.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
        drawPhong<OpaqueComponent>(ecs, cameraEntity, light);

        drawFXAA(backbuffer, *sceneTarget->mTextures[0]);
    }

    void Demo::destroy() {
    }

}