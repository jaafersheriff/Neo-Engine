#include "Froxels/Froxels.hpp"
#include "VolumeComponent.hpp"
#include "VolumeWriteCameraComponent.hpp"

#include "VolumeDebugShader.hpp"
#include "VolumeWriteShader.hpp"
#include "PhongShader.hpp"

#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/AlphaTestRenderable.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/PhongRenderable.hpp"
#include "ECS/Component/RenderableComponent/LineMeshComponent.hpp"
#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustaFittingSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumToLineSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/AlphaTestShader.hpp"
#include "Renderer/Shader/WireFrameShader.hpp"
#include "Renderer/Shader/LineShader.hpp"

#include "Renderer/GLObjects/Material.hpp"
#include "Renderer/GLObjects/Texture3D.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

namespace Froxels {

    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "Froxels Demo";
        config.shaderDir = "shaders/froxels/";
        return config;
    }

    void Demo::init(ECS& ecs, Renderer& renderer) {

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
            ecs.addComponent<LightComponent>(entity, glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
        }

        {
            /* Bunny object */
            auto bunny = ecs.createEntity();
            ecs.addComponent<TagComponent>(bunny, "Bunny");
            ecs.addComponent<SpatialComponent>(bunny, glm::vec3(0.f, 1.0f, 0.f));
            ecs.addComponent<RotationComponent>(bunny, glm::vec3(0.f, 1.0f, 0.f));
            ecs.addComponent<MeshComponent>(bunny, Library::loadMesh("bunny.obj", true).mMesh);
            ecs.addComponent<renderable::PhongRenderable>(bunny, Library::getTexture("black"), Material(glm::vec3(0.2f), glm::vec3(1.f, 0.f, 1.f)));
            ecs.addComponent<SelectableComponent>(bunny);
            ecs.addComponent<BoundingBoxComponent>(bunny, Library::loadMesh("bunny.obj"));
        }

        // Voxel stuffs
        {
            {
                auto entity = ecs.createEntity();
                TextureFormat format;
                format.mInternalFormat = GL_RGBA32F;
                format.mBaseFormat = GL_RGBA;
                format.mFilter = GL_LINEAR;
                format.mMode = GL_CLAMP_TO_EDGE;
                format.mType = GL_FLOAT;
                ecs.addComponent<VolumeComponent>(entity, Library::createEmptyTexture<Texture3D>("Volume", format, { 8, 8, 8 }));
                ecs.addComponent<TagComponent>(entity, "Volume");
                ecs.addComponent<OrthoCameraComponent>(entity, -2.f, 2.f, -4.f, 2.f, 0.1f, 5.f);
                ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f), glm::vec3(1.f));
                ecs.addComponent<FrustumComponent>(entity);
                ecs.addComponent<FrustumFitReceiverComponent>(entity);
                ecs.addComponent<LineMeshComponent>(entity);
            }

            {
                auto entity = ecs.createEntity();
                ecs.addComponent<TagComponent>(entity, "Mock camera");
                auto spat = ecs.addComponent<SpatialComponent>(entity, glm::vec3(7.f, 4.f, 2.f), glm::vec3(1.f));
                spat->setLookDir(glm::vec3(-1.f, 0.f, 0.f));
                ecs.addComponent<PerspectiveCameraComponent>(entity, 0.1f, 10.f, 45.f);
                ecs.addComponent<VolumeWriteCameraComponent>(entity);
                ecs.addComponent<LineMeshComponent>(entity, glm::vec3(0.f, 1.f, 1.f));
                ecs.addComponent<FrustumComponent>(entity);
                ecs.addComponent<FrustumFitSourceComponent>(entity);
            }


        }

        /* Ground plane */
        auto plane = ecs.createEntity();
        ecs.addComponent<TagComponent>(plane, "Grid");
        ecs.addComponent<SpatialComponent>(plane, glm::vec3(0.f), glm::vec3(15.f), glm::vec3(-util::PI / 2.f, 0.f, 0.f));
        ecs.addComponent<MeshComponent>(plane, Library::getMesh("quad").mMesh);
        ecs.addComponent<renderable::AlphaTestRenderable>(plane, Library::loadTexture("grid.png"));

        /* Systems - order matters! */
        ecs.addSystem<CameraControllerSystem>();
        ecs.addSystem<RotationSystem>();
        ecs.addSystem<FrustumSystem>();
        ecs.addSystem<FrustaFittingSystem>();
        ecs.addSystem<FrustumToLineSystem>();

        /* Init renderer */
        auto& compute = renderer.addComputeShader<VolumeWriteShader>("volumewrite.compute");
        compute.mActive = false;
        renderer.addPreProcessShader<Froxels::PhongShader>();
        renderer.addSceneShader<neo::PhongShader>();
        renderer.addSceneShader<AlphaTestShader>();
        renderer.addSceneShader<WireframeShader>();
        renderer.addSceneShader<LineShader>();
        auto& debug = renderer.addSceneShader<VolumeDebugShader>("voxel.vert", "voxel.frag");
        debug.mActive = false;
    }

    void Demo::destroy() {
    }

    void Demo::update(ECS& ecs) {
        NEO_UNUSED(ecs);
    }

    void Demo::imGuiEditor(ECS& ecs) {
        if (auto volume = ecs.getSingleView<VolumeComponent, TagComponent>()) {
            auto&& [_, vol, spat] = *volume;
            vol.imGuiEditor();
        }

        if (auto mockCamera = ecs.getSingleView<VolumeWriteCameraComponent, PerspectiveCameraComponent, SpatialComponent>()) {
            auto&& [_, __, camera, spatial] = *mockCamera;
            camera.imGuiEditor();
            spatial.imGuiEditor();
        }
    }

}
