#include "Base/BaseDemo.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"
#include "ECS/Component/RenderingComponent/PhongShaderComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */

namespace Base {

    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "Base Demo";
        return config;
    }

    void Demo::init(ECS& ecs) {

        /* Camera */
        {
            auto entity = ecs.createEntity();
            ecs.addComponent<TagComponent>(entity, "Camera");
            ecs.addComponent<SpatialComponent>(entity, glm::vec3(0, 0.6f, 5), glm::vec3(1.f));
            ecs.addComponent<PerspectiveCameraComponent>(entity, 1.f, 100.f, 45.f);
            ecs.addComponent<CameraControllerComponent>(entity, 0.4f, 7.f);
            ecs.addComponent<MainCameraComponent>(entity);
        }

        {
            auto entity = ecs.createEntity();
            ecs.addComponent<TagComponent>(entity, "Light");
            ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 2.f, 20.f));
            ecs.addComponent<LightComponent>(entity, glm::vec3(1.f));
            ecs.addComponent<PointLightComponent>(entity, glm::vec3(0.1, 0.05, 0.003f));
        }

        /* Bunny object */
        {
            auto bunny = ecs.createEntity();
            ecs.addComponent<TagComponent>(bunny, "Bunny");
            ecs.addComponent<SpatialComponent>(bunny, glm::vec3(0.f, 1.0f, 0.f));
            ecs.addComponent<RotationComponent>(bunny, glm::vec3(0.f, 1.0f, 0.f));
            ecs.addComponent<MeshComponent>(bunny, Library::loadMesh("bunny.obj", true).mMesh);
            ecs.addComponent<BoundingBoxComponent>(bunny, Library::loadMesh("bunny.obj"));
            ecs.addComponent<PhongShaderComponent>(bunny);
            ecs.addComponent<OpaqueComponent>(bunny);
            auto material = ecs.addComponent<MaterialComponent>(bunny);
            material->mAmbient = glm::vec3(0.2f);
            material->mDiffuse = glm::vec3(1.f, 0.f, 1.f);
            material->mSpecular = glm::vec3(1.f);
            material->mShininess = 20.f;
        }

        /* Ground plane */
        {
            auto plane = ecs.createEntity();
            ecs.addComponent<TagComponent>(plane, "Grid");
            ecs.addComponent<SpatialComponent>(plane, glm::vec3(0.f), glm::vec3(15.f), glm::vec3(-util::PI / 2.f, 0.f, 0.f));
            ecs.addComponent<MeshComponent>(plane, Library::getMesh("quad").mMesh);
            ecs.addComponent<PhongShaderComponent>(plane);
            ecs.addComponent<AlphaTestComponent>(plane);
            auto material = ecs.addComponent<MaterialComponent>(plane);
            material->mAmbient = glm::vec3(1.f);
            material->mDiffuseMap = Library::loadTexture("grid.png");
        }

        /* Systems - order matters! */
        ecs.addSystem<CameraControllerSystem>();
        ecs.addSystem<RotationSystem>();

    }

    void Demo::imGuiEditor(ECS& ecs) {
        NEO_UNUSED(ecs);
    }

    void Demo::update(ECS& ecs) {
        NEO_UNUSED(ecs);
    }

    void Demo::render(const ECS& ecs, Framebuffer& backbuffer) {
        const auto&& [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();

        auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
        auto sceneTarget = Library::createTransientFBO(viewport.mSize, {
            TextureFormat{
                TextureTarget::Texture2D,
                GL_RGB8,
                GL_RGB,
            },
            TextureFormat{
                TextureTarget::Texture2D,
                GL_DEPTH_COMPONENT,
                GL_DEPTH_COMPONENT,
            }
        });

        glm::vec3 clearColor = getConfig().clearColor;

        sceneTarget->bind();
        sceneTarget->clear(glm::vec4(clearColor, 1.f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
        drawPhong<OpaqueComponent>(ecs, cameraEntity);
        drawPhong<AlphaTestComponent>(ecs, cameraEntity);

        backbuffer.bind();
        backbuffer.clear(glm::vec4(clearColor, 1.f), GL_COLOR_BUFFER_BIT);
        drawFXAA(backbuffer, *sceneTarget->mTextures[0]);
    }

    void Demo::destroy() {
    }
}
