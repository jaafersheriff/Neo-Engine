#include "Deferred/Deferred.hpp"
#include "Engine/Engine.hpp"

#include "Renderer/GLObjects/Material.hpp"
#include "Renderer/Shader/WireframeShader.hpp"
#include "DecalShader.hpp"
#include "GBufferShader.hpp"
#include "LightPassShader.hpp"
#include "AOShader.hpp"
#include "CombineShader.hpp"
#include "BlurShader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/TransformationComponent/RotationComponent.hpp"
#include "ECS/Component/TransformationComponent/SinTranslateComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"
#include "ECS/Systems/TranslationSystems/SinTranslateSystem.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "Util/Util.hpp"

using namespace neo;

/* Game object definitions */
namespace Deferred {

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
        GameObject* gameObject;

        Light(ECS& ecs, glm::vec3 pos, glm::vec3 col, glm::vec3 scale) {
            gameObject = &ecs.createGameObject();
            ecs.addComponent<SpatialComponent>(gameObject, pos, scale);
            ecs.addComponent<LightComponent>(gameObject, col);
        }
    };

    struct Renderable {
        GameObject* gameObject;

        Renderable(ECS& ecs, Mesh* mesh, glm::vec3 pos, glm::vec3 scale, glm::mat3 rot = glm::mat3(1.f)) {
            gameObject = &ecs.createGameObject();
            auto& spat = ecs.addComponent<SpatialComponent>(gameObject, pos, scale);
            spat.setOrientation(rot);
            ecs.addComponent<MeshComponent>(gameObject, *mesh);
        }
    };

    struct Decal {
        GameObject* gameObject;

        Decal(ECS& ecs, Texture* texture, glm::vec3 pos, glm::vec3 scale) {
            gameObject = &ecs.createGameObject();
            ecs.addComponent<SpatialComponent>(gameObject, pos, scale);
            ecs.addComponent<DecalRenderable>(gameObject, *texture);
            ecs.addComponent<RotationComponent>(gameObject, glm::vec3(0, 1, 0));
        }
    };

    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "Deferred";
        config.shaderDir = "shaders/deferred/";
        return config;
    }

    void Demo::init(ECS& ecs) {

        /* Game objects */
        Camera camera(ecs, 45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 20.f);
        ecs.addComponent<MainCameraComponent>(&camera.camera->getGameObject());

        Light(ecs, glm::vec3(25.f, 25.f, 0.f), glm::vec3(1.f), glm::vec3(100.f));
        {
            Renderable cube(ecs, Library::getMesh("cube").mMesh, glm::vec3(10.f, 0.75f, 0.f), glm::vec3(5.f));
            Material material;
            material.mAmbient = glm::vec3(0.2f);
            material.mDiffuse = util::genRandomVec3();
            ecs.addComponent<GBufferComponent>(cube.gameObject, *Library::getTexture("black"), material);
        }
        {
            Renderable dragon(ecs, Library::loadMesh("dragon10k.obj", true).mMesh, glm::vec3(-4.f, 5.f, -5.f), glm::vec3(10.f));
            Material material;
            material.mAmbient = glm::vec3(0.2f);
            material.mDiffuse = util::genRandomVec3();
            ecs.addComponent<GBufferComponent>(dragon.gameObject, *Library::getTexture("black"), material);
        }
        {
            Renderable stairs(ecs, Library::loadMesh("staircase.obj", true).mMesh, glm::vec3(5.f, 5.f, 9.f), glm::vec3(10.f));
            Material material;
            material.mAmbient = glm::vec3(0.2f);
            material.mDiffuse = util::genRandomVec3();
            ecs.addComponent<GBufferComponent>(stairs.gameObject, *Library::getTexture("black"), material);
        }
        Library::loadMesh("PineTree3.obj", true);
        Library::loadTexture("PineTexture.png");
        for (int i = 0; i < 20; i++) {
            Renderable tree(ecs, Library::getMesh("PineTree3.obj").mMesh, glm::vec3(50.f - i * 5.f, 5.f, 25.f + 25.f * util::genRandom()), glm::vec3(10.f));
            Material material;
            material.mAmbient = glm::vec3(0.2f);
            material.mDiffuse = glm::vec3(0.f);
            ecs.addComponent<GBufferComponent>(tree.gameObject, *Library::getTexture("PineTexture.png"), material);
        }

        // Terrain 
        Renderable terrain(ecs, Library::getMesh("quad").mMesh, glm::vec3(0.f, 0.f, 0.f), glm::vec3(1000.f), glm::mat3(glm::rotate(glm::mat4(1.f), -1.56f, glm::vec3(1, 0, 0))));
        Material material;
        material.mAmbient = glm::vec3(0.7f);
        material.mDiffuse = glm::vec3(0.7f);
        ecs.addComponent<GBufferComponent>(terrain.gameObject, *Library::getTexture("black"), material);

        Decal decal(ecs, Library::loadTexture("decal.png"), glm::vec3(5.f, 0.f, 0.f), glm::vec3(15.f, 30.f, 15.f));

        /* Systems - order matters! */
        ecs.addSystem<CameraControllerSystem>();
        ecs.addSystem<RotationSystem>();
        ecs.addSystem<SinTranslateSystem>();

        // TODO - this ordering is super broken
        Renderer::addPreProcessShader<GBufferShader>("gbuffer.vert", "gbuffer.frag");
        Renderer::addPreProcessShader<DecalShader>("decal.vert", "decal.frag");
        Renderer::addPreProcessShader<LightPassShader>("lightpass.vert", "lightpass.frag");  // run light pass after generating gbuffer
        Renderer::addPostProcessShader<AOShader>("ao.frag");    // first post process - generate ssao map 
        Renderer::addPostProcessShader<BlurShader>("blur.frag"); // blur ssao map
        Renderer::addPostProcessShader<CombineShader>("combine.frag");    // combine light pass and ssao 

    }

    void Demo::imGuiEditor(ECS& ecs) {

        /* Attach ImGui panes */
        if(ImGui::TreeNodeEx("Decal")) {
            auto decal = ecs.getComponentTuple<DecalRenderable, SpatialComponent>();
            auto spat = decal->get<SpatialComponent>();
            auto pos = spat->getPosition();
            auto scale = spat->getScale();
            if (ImGui::SliderFloat3("Position", &pos[0], -25.f, 25.f)) {
                spat->setPosition(pos);
            }
            if (ImGui::SliderFloat3("Scale", &scale[0], 0.f, 50.f)) {
                spat->setScale(scale);
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Lights")) {
            static int index;
            if (ImGui::CollapsingHeader("Create Lights")) {
                if (ImGui::TreeNode("Single")) {
                    static glm::vec3 pos(0.f);
                    static float size(15.f);
                    static glm::vec3 color(1.f);
                    static float yOffset(10.f);
                    ImGui::SliderFloat3("Position", &pos[0], -25.f, 25.f);
                    ImGui::SliderFloat("Scale", &size, 15.f, 100.f);
                    ImGui::SliderFloat3("Color", &color[0], 0.01f, 1.f);
                    ImGui::SliderFloat("Offset", &yOffset, 0.f, 25.f);
                    if (ImGui::Button("Create")) {
                        auto light = Light(ecs, pos, color, glm::vec3(size));
                        if (yOffset) {
                            ecs.addComponent<SinTranslateComponent>(light.gameObject, glm::vec3(0.f, yOffset, 0.f), pos);
                        }
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Random Lights")) {
                    if (ImGui::Button("Clear lights")) {
                        for (auto& l : ecs.getComponents<LightComponent>()) {
                            ecs.removeGameObject(l->getGameObject());
                        }
                    }
                    static int numLights = 10;
                    static glm::vec3 minOffset(0.f);
                    static glm::vec3 maxOffset(0.f);
                    static float minScale(0.f);
                    static float maxScale(10.f);
                    static float minSinOffset(0.f);
                    static float maxSinOffset(0.f);
                    ImGui::SliderInt("Num lights", &numLights, 0, 1000);
                    ImGui::SliderFloat3("Min offset", &minOffset[0], -100.f, 100.f);
                    ImGui::SliderFloat3("Max offset", &maxOffset[0], -100.f, 100.f);
                    ImGui::SliderFloat("Min scale", &minScale, 0.f, maxScale);
                    ImGui::SliderFloat("Max scale", &maxScale, minScale, 100.f);
                    ImGui::SliderFloat("Min sin", &minSinOffset, 0.f, 15.f);
                    ImGui::SliderFloat("Max sin", &maxSinOffset, 0.f, 15.f);
                    if (ImGui::Button("Create light")) {
                        for (int i = 0; i < numLights; i++) {
                            glm::vec3 position = glm::vec3(
                                util::genRandom(minOffset.x, maxOffset.x),
                                util::genRandom(minOffset.y, maxOffset.y),
                                util::genRandom(minOffset.z, maxOffset.z)
                            );
                            glm::vec3 color = util::genRandomVec3();
                            float size = util::genRandom(minScale, maxScale);
                            auto light = Light(ecs, position, color, glm::vec3(size));
                            ecs.addComponent<SinTranslateComponent>(light.gameObject, glm::vec3(0.f, util::genRandom(minSinOffset, maxSinOffset), 0.f), position);
                        }
                    }
                    ImGui::TreePop();
                }
            }
            auto lights = ecs.getComponents<LightComponent>();
            if (lights.empty()) {
                return;
            }
            if (ImGui::CollapsingHeader("Edit Lights")) {
                ImGui::SliderInt("Index", &index, 0, static_cast<int>(lights.size()) - 1);
                auto l = lights[index];
                if (ImGui::Button("Delete light")) {
                    ecs.removeGameObject(l->getGameObject());
                    lights.erase(lights.begin() + index);
                    index = glm::max(0, index - 1);
                }
                l->imGuiEditor();
                auto spat = l->getGameObject().getComponentByType<SpatialComponent>();
                if (!spat) {
                    return;
                }
                spat->imGuiEditor();
            }

            ImGui::TreePop();
        }
    }
}
