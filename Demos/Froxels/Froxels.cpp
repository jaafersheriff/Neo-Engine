#include "Froxels/Froxels.hpp"
#include "VolumeComponent.hpp"

#include "VolumeDebugShader.hpp"

#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/AlphaTestRenderable.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/PhongRenderable.hpp"
#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/AlphaTestShader.hpp"
#include "Renderer/Shader/WireFrameShader.hpp"

#include "Renderer/GLObjects/Material.hpp"
#include "Renderer/GLObjects/Texture3D.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
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

        // Voxel stuffs
        {
            auto entity = ecs.createEntity();
            TextureFormat format;
            format.mInternalFormat = GL_RGBA8;
            format.mBaseFormat = GL_RGBA;
            format.mFilter = GL_LINEAR;
            format.mMode = GL_CLAMP_TO_EDGE;
            format.mType = GL_UNSIGNED_BYTE;
            ecs.addComponent<VolumeComponent>(entity, Library::createEmptyTexture<Texture3D>("Volume", format, { 8, 8, 8 }));
            ecs.addComponent<TagComponent>(entity, "Volume");
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

        /* Init renderer */
        renderer.addSceneShader<PhongShader>();
        renderer.addSceneShader<AlphaTestShader>();
        renderer.addSceneShader<WireframeShader>();
        renderer.addSceneShader<VolumeDebugShader>("voxel.vert", "voxel.frag");
    }

    void Demo::update(ECS& ecs) {
        NEO_UNUSED(ecs);
    }

    void Demo::destroy() {
    }

    void Demo::imGuiEditor(ECS& ecs) {
        if (auto volume = ecs.getSingleView<VolumeComponent, TagComponent>()) {
            auto&& [_, vol, spat] = *volume;
            if (ImGui::Button("Randomize Volume")) {
                std::vector<uint32_t> data;
                data.resize(vol.mTexture->mWidth * vol.mTexture->mHeight * vol.mTexture->mDepth);
                for (int i = 0; i < data.size(); i++) {
                    float r = util::genRandom();
                    data[i] = *reinterpret_cast<uint32_t*>(&r);
                }
                vol.mTexture->update({vol.mTexture->mWidth, vol.mTexture->mHeight, vol.mTexture->mDepth}, data.data());
            }
        }
    }

}
