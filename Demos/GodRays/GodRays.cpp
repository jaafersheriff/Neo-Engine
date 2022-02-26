#include "GodRays/GodRays.hpp"
#include "Engine/Engine.hpp"

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/AlphaTestShader.hpp"
#include "Renderer/Shader/GammaCorrectShader.hpp"

#include "SunComponent.hpp"
#include "SunOccluderComponent.hpp"
#include "GodRaySunShader.hpp"
#include "GodRayOccluderShader.hpp"
#include "BlurShader.hpp"
#include "CombineShader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
namespace GodRays {
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
        Light(ECS& ecs, glm::vec3 pos, float scale, glm::vec3 col, glm::vec3 att) {
            auto& gameObject = ecs.createGameObject();
            ecs.addComponent<SpatialComponent>(&gameObject, pos, glm::vec3(scale));
            ecs.addComponent<LightComponent>(&gameObject, col, att);
            ecs.addComponent<SunComponent>(&gameObject);
            ecs.addComponent<SelectableComponent>(&gameObject);
        }
    };

    struct Renderable {
        GameObject* gameObject;

        Renderable(ECS& ecs, Mesh* mesh, glm::vec3 position = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f), glm::vec3 rotation = glm::vec3(0.f)) {
            gameObject = &ecs.createGameObject();
            ecs.addComponent<MeshComponent>(gameObject, *mesh);
            ecs.addComponent<SpatialComponent>(gameObject, position, scale, rotation);
        }
    };

    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "GodRays";
        config.clearColor = { 0.f, 0.f, 0.f };
        return config;
    }

    void Demo::init(ECS& ecs) {

        /* Game objects */
        Camera camera(ecs, 45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
        ecs.addComponent<MainCameraComponent>(&camera.camera->getGameObject());

        Light(ecs, glm::vec3(0.f, 2.f, -20.f), 12.f, glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

        /* Trees */
        Library::loadTexture("PineTexture.png");
        Library::loadMesh("PineTree3.obj");
        for (int i = 0; i < 15; i++) {
            Renderable cube(ecs, Library::getMesh("PineTree3.obj").mMesh, glm::vec3(util::genRandom(-7.5f, 7.5f), 0.5f, util::genRandom(-7.5f, 7.5f)), glm::vec3(util::genRandom(0.7f, 1.3f)), glm::vec3(0.f, util::genRandom(0.f, 360.f), 0.f));
            Material material;
            material.mAmbient = glm::vec3(0.2f);
            material.mDiffuse = glm::vec3(0.f);
            ecs.addComponent<renderable::PhongRenderable>(cube.gameObject, *Library::getTexture("PineTexture.png"), material);
            ecs.addComponent<SunOccluderComponent>(cube.gameObject, *Library::getTexture("PineTexture.png"));
        }

        /* Ground plane */
        Renderable plane(ecs, Library::getMesh("quad").mMesh, glm::vec3(0.f), glm::vec3(15.f), glm::vec3(-util::PI / 2.f, 0.f, 0.f));
        ecs.addComponent<renderable::AlphaTestRenderable>(plane.gameObject, *Library::loadTexture("grid.png"));

        /* Systems - order matters! */
        ecs.addSystem<CameraControllerSystem>();
        ecs.addSystem<RotationSystem>();

        /* Init renderer */
        Renderer::addPreProcessShader<GodRaySunShader>("godrays/billboard.vert", "godrays/godraysun.frag");
        Renderer::addPreProcessShader<GodRayOccluderShader>("godrays/model.vert", "godrays/godrayoccluder.frag");
        Renderer::addPreProcessShader<BlurShader>("godrays/blur.vert", "godrays/blur.frag");
        Renderer::addSceneShader<PhongShader>();
        Renderer::addSceneShader<AlphaTestShader>();
        Renderer::addPostProcessShader<CombineShader>("godrays/combine.frag");
        Renderer::addPostProcessShader<GammaCorrectShader>();
    }

    void Demo::imGuiEditor(ECS& ecs) {
        auto light = ecs.getSingleComponent<LightComponent>();
        if (auto spatial = light->getGameObject().getComponentByType<SpatialComponent>()) {
            glm::vec3 pos = spatial->getPosition();
            if (ImGui::SliderFloat3("Position", &pos[0], -100.f, 100.f)) {
                spatial->setPosition(pos);
            }
            float scale = spatial->getScale().x;
            if (ImGui::SliderFloat("Scale", &scale, 0.f, 100.f)) {
                spatial->setScale(glm::vec3(scale));
            }
            ImGui::SliderFloat3("Color", &light->mColor[0], 0.f, 1.f);
            ImGui::SliderFloat3("Attenuation", &light->mAttenuation[0], 0.f, 1.f);
        }
    }
}