#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "GBufferComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/AlphaTestRenderable.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/PhongRenderable.hpp"
#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/TransformationComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "DecalShader.hpp"
#include "GBufferShader.hpp"
#include "LightPassShader.hpp"
#include "CombineShader.hpp"
#include "Renderer/Shader/GammaCorrectShader.hpp"

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

struct Decal {
    GameObject *gameObject;
    SpatialComponent *spat;

    Decal(ECS& ecs, Texture* texture, glm::vec3 pos, glm::vec3 scale) {
        gameObject = &ecs.createGameObject();
        spat = &ecs.addComponent<SpatialComponent>(gameObject, pos, scale);
        ecs.addComponent<DecalRenderable>(gameObject, *texture);
        ecs.addComponent<RotationComponent>(gameObject, glm::vec3(0, 1, 0));
        Engine::addImGuiFunc("Decal", [this](ECS& ecs_) {
            NEO_UNUSED(ecs_);
            auto pos = spat->getPosition();
            auto scale = spat->getScale();
            if (ImGui::SliderFloat3("Position", &pos[0], -25.f, 25.f)) {
                spat->setPosition(pos);
            }
            if (ImGui::SliderFloat3("Scale", &scale[0], 0.f, 50.f)) {
                spat->setScale(scale);
            }
        });

    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "DeferredDecal";
    config.APP_RES = "res/";
    ECS& ecs = Engine::init(config);

    /* Game objects */
    Camera camera(ecs, 45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 20.f);
    ecs.addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    std::vector<Light *> lights;
    lights.push_back(new Light(ecs, glm::vec3(25.f, 25.f, 0.f), glm::vec3(1.f), glm::vec3(100.f)));

    Renderable cube(ecs, Library::getMesh("cube"), Library::getTexture("black"), glm::vec3(10.f, 0.75f, 0.f), glm::vec3(5.f));
    Renderable dragon(ecs, Library::loadMesh("dragon10k.obj", true), Library::getTexture("black"), glm::vec3(-4.f, 10.f, -5.f), glm::vec3(10.f));
    Renderable stairs(ecs, Library::loadMesh("staircase.obj", true), Library::getTexture("black"), glm::vec3(5.f, 10.f, 9.f), glm::vec3(10.f));

    for (int i = 0; i < 20; i++) {
        Renderable tree(ecs, Library::loadMesh("PineTree3.obj"), Library::loadTexture("PineTexture.png"), glm::vec3(50.f - i * 5.f, 10.f, 25.f + 25.f * util::genRandom()), glm::vec3(10.f));
        tree.gbuffer->mMaterial.mDiffuse = glm::vec3(0.f);
    }

    // Terrain 
    Renderable terrain(ecs, Library::getMesh("quad"), Library::getTexture("black"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(1000.f));
    terrain.spatial->rotate(glm::mat3(glm::rotate(glm::mat4(1.f), -1.56f, glm::vec3(1, 0, 0))));
    terrain.gbuffer->mMaterial.mAmbient = glm::vec3(0.7f);
    terrain.gbuffer->mMaterial.mDiffuse = glm::vec3(0.7f);

    Decal decal(ecs, Library::loadTexture("texture.png"), glm::vec3(0.f, 0.f, -25.f), glm::vec3(15.f));

    /* Systems - order matters! */
    ecs.addSystem<CameraControllerSystem>();
    ecs.addSystem<RotationSystem>();

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addPreProcessShader<GBufferShader>("gbuffer.vert", "gbuffer.frag");
    Renderer::addPreProcessShader<DecalShader>("decal.vert", "decal.frag"); 
    Renderer::addPreProcessShader<LightPassShader>("lightpass.vert", "lightpass.frag"); 
    Renderer::addPostProcessShader<CombineShader>("combine.frag"); 
    Renderer::addPostProcessShader<GammaCorrectShader>();

    /* Attach ImGui panes */
    Engine::addImGuiFunc("Lights", [&lights](ECS& ecs_) {
 
        static int index;
        if (ImGui::CollapsingHeader("Create Lights")) {
            static glm::vec3 pos(0.f);
            static float size(15.f);
            static glm::vec3 color(1.f);
            static float yOffset(10.f);
            ImGui::SliderFloat3("Position", &pos[0], -25.f, 25.f);
            ImGui::SliderFloat("Scale", &size, 15.f, 100.f);
            ImGui::SliderFloat3("Color", &color[0], 0.01f, 1.f);
            ImGui::SliderFloat("Offset", &yOffset, 0.f, 25.f);
            if (ImGui::Button("Create")) {
                auto light = new Light(ecs_, pos, color, glm::vec3(size));
                lights.push_back(light);
                index = static_cast<int>(lights.size()) - 1;
            }
        }
        if (lights.empty()) {
            return;
        }
        if (ImGui::CollapsingHeader("Edit Lights")) {
            ImGui::SliderInt("Index", &index, 0, static_cast<int>(lights.size()) - 1);
            auto l = lights[index];
            if (ImGui::Button("Delete light")) {
                ecs_.removeGameObject(*l->gameObject);
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