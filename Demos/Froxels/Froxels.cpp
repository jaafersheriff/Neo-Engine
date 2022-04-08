#include "Froxels/Froxels.hpp"
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

    class Volume {
    public:
        Volume(Texture* tex)
            : mTexture(tex)
        {
        }
    private:
        Texture* mTexture;
    };

    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "Froxels Demo";
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
            for (int i = 0; i < 25; i++) {
                auto entity = ecs.createEntity();
                ecs.addComponent<SpatialComponent>(entity, glm::vec3(util::genRandom(-7.f, 7.f), util::genRandom(0.f, 5.f), util::genRandom(-7.f, 7.f)), glm::vec3(util::genRandom(1.f, 2.f)));
                ecs.addComponent<MeshComponent>(entity, Library::getMesh("sphere").mMesh);
                ecs.addComponent<renderable::WireframeRenderable>(entity, util::genRandomVec3(0.3f, 1.f));
            }

            auto entity = ecs.createEntity();
            TextureFormat format;
            format.mInternalFormat = GL_RGBA8;
            format.mBaseFormat = GL_RGBA;
            format.mFilter = GL_NEAREST;
            format.mMode = GL_REPEAT;
            format.mType = GL_UNSIGNED_BYTE;
            ecs.addComponent<Volume>(entity, Library::createEmptyTexture<Texture3D>("Volume", format, { 8, 8, 8 }));
            ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f), glm::vec3(1.f));
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
    }

    void Demo::update(ECS& ecs) {
        NEO_UNUSED(ecs);
    }

    void Demo::destroy() {
    }

    void Demo::imGuiEditor(ECS& ecs) {
        if (auto volume = ecs.getSingleView<Volume, SpatialComponent>()) {
            auto&& [_, vol, spat] = *volume;
        }
    }

}
