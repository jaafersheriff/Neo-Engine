#include "Engine/Engine.hpp"

#include "GBufferShader.hpp"
#include "LightPassShader.hpp"
#include "CombineShader.hpp"
#include "Renderer/Shader/WireframeShader.hpp"
#include "Renderer/Shader/GammaCorrectShader.hpp"

#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/TransformationComponent/SinTranslateComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/SinTranslateSystem.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "Util/Util.hpp"

using namespace neo;

/* Game object definitions */
struct Camera {
    CameraComponent *camera;
    Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        GameObject *gameObject = &ecs.createGameObject();
        ecs.addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &ecs.addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
        ecs.addComponent<CameraControllerComponent>(gameObject, ls, ms);
    }
};

struct Light {
    GameObject *gameObject;
    LightComponent *light;

    Light(ECS& ecs, glm::vec3 pos, glm::vec3 col, glm::vec3 scale) {
        gameObject = &ecs.createGameObject();
        ecs.addComponent<SpatialComponent>(gameObject, pos, scale);
        light = &ecs.addComponent<LightComponent>(gameObject, col);
    }
};

struct Renderable {
    GameObject *gameObject;
    SpatialComponent *spatial;
    GBufferComponent *gbuffer;

    Renderable(ECS& ecs, Mesh *mesh, Texture* texture, glm::vec3 pos, glm::vec3 scale) {
        gameObject = &ecs.createGameObject();
        spatial = &ecs.addComponent<SpatialComponent>(gameObject, pos, scale);
        ecs.addComponent<MeshComponent>(gameObject, *mesh);
        Material material;
        material.mAmbient = glm::vec3(0.2f);
        material.mDiffuse = util::genRandomVec3();
        gbuffer = &ecs.addComponent<GBufferComponent>(gameObject, *texture, material);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Deferred";
    config.APP_RES = "res/";
    ECS& ecs = Engine::init(config);

    /* Game objects */
    Camera camera(ecs, 45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 20.f);
    ecs.addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    /* Lights */
    std::vector<Light *> lights;
    lights.push_back(new Light(ecs, glm::vec3(25.f, 25.f, 0.f), glm::vec3(1.f), glm::vec3(100.f)));

    /* Renderables */
    Renderable cube(ecs, Library::getMesh("cube"), Library::getTexture("black"), glm::vec3(10.f, 0.75f, 0.f), glm::vec3(5.f));
    Renderable dragon(ecs, Library::loadMesh("dragon10k.obj", true), Library::getTexture("black"), glm::vec3(-4.f, 10.f, -5.f), glm::vec3(10.f));
    Renderable stairs(ecs, Library::loadMesh("staircase.obj", true), Library::getTexture("black"), glm::vec3(5.f, 10.f, 9.f), glm::vec3(10.f));

    Texture* pineTexture = Library::loadTexture("PineTexture.png");
    for (int i = 0; i < 20; i++) {
        Renderable tree(ecs, Library::loadMesh("PineTree3.obj", true), pineTexture, glm::vec3(50.f - i * 5.f, 10.f, 25.f + 25.f * util::genRandom()), glm::vec3(10.f));
        tree.gbuffer->mMaterial.mDiffuse = glm::vec3(0.f);
    }

    // Terrain 
    Renderable terrain(ecs, Library::getMesh("quad"), Library::getTexture("black"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(1000.f));
    terrain.spatial->rotate(glm::mat3(glm::rotate(glm::mat4(1.f), -1.56f, glm::vec3(1, 0, 0))));
    terrain.gbuffer->mMaterial.mAmbient = glm::vec3(0.7f);
    terrain.gbuffer->mMaterial.mDiffuse = glm::vec3(0.7f);

    /* Systems - order matters! */
    ecs.addSystem<CameraControllerSystem>();
    ecs.addSystem<SinTranslateSystem>();

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addPreProcessShader<GBufferShader>("gbuffer.vert", "gbuffer.frag");
    Renderer::addPreProcessShader<LightPassShader>("lightpass.vert", "lightpass.frag"); 
    Renderer::addPostProcessShader<CombineShader>("combine.frag"); 
    Renderer::addPostProcessShader<GammaCorrectShader>();

    /* Attach ImGui panes */
    Engine::addImGuiFunc("Lights", [&lights](ECS& ecs_) {
        static int index;
        if (ImGui::CollapsingHeader("Create Lights")) {
            if (ImGui::TreeNode("Single")) {
                static glm::vec3 pos(0.f);
                static float size(15.f);
                static glm::vec3 color(1.f);
                static glm::vec3 yOffset(0.f, 10.f, 0.f);
                ImGui::SliderFloat3("Position", &pos[0], -25.f, 25.f);
                ImGui::SliderFloat("Scale", &size, 15.f, 100.f);
                ImGui::SliderFloat3("Color", &color[0], 0.01f, 1.f);
                ImGui::SliderFloat("Offset", &yOffset.y, 0.f, 25.f);
                if (ImGui::Button("Create")) {
                    auto light = new Light(ecs_, pos, color, glm::vec3(size));
                    if (yOffset.y) {
                        ecs_.addComponent<SinTranslateComponent>(light->gameObject, yOffset, pos);
                    }
                    lights.push_back(light);
                    index = static_cast<int>(lights.size()) - 1;
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Random Lights")) {
                if (ImGui::Button("Clear lights")) {
                    for (auto & l : lights) {
                        ecs_.removeGameObject(*l->gameObject);
                    }
                    lights.clear();
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
                        auto light = new Light(ecs_, position, color, glm::vec3(size));
                        glm::vec3 sinMove(0.f, util::genRandom(minSinOffset, maxSinOffset), 0.f);
                        ecs_.addComponent<SinTranslateComponent>(light->gameObject, sinMove, position);
                        lights.push_back(light);
                    }
                }
                ImGui::TreePop();
            }
        }
        if (lights.empty()) {
            return;
        }
        if (ImGui::CollapsingHeader("Edit Lights")) {
            ImGui::SliderInt("Index", &index, 0, static_cast<int>(lights.size()) - 1);
            auto l = lights[index];
            if (ImGui::Button("Delete light")) {
                ecs.removeGameObject(*l->gameObject);
                lights.erase(lights.begin() + index);
                index = glm::max(0, index - 1);
            }
            l->light->imGuiEditor();
            if (auto spatial = l->gameObject->getComponentByType<SpatialComponent>()) {
                spatial->imGuiEditor();
            }
        }
    });

    /* Run */
    Engine::run();
    return 0;
}