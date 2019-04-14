#include <NeoEngine.hpp>

#include "DecalShader.hpp"
#include "GBufferShader.hpp"
#include "LightPassShader.hpp"
#include "CombineShader.hpp"
#include "Shader/WireframeShader.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "Util/Util.hpp"

using namespace neo;

/* Game object definitions */
struct Camera {
    CameraComponent *camera;
    Camera(float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        GameObject *gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &NeoEngine::addComponent<CameraComponent>(gameObject, fov, near, far);
        NeoEngine::addComponent<CameraControllerComponent>(gameObject, ls, ms);
    }
};

struct Light {
    GameObject *gameObject;
    LightComponent *light;

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 scale) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos, scale);
        light = &NeoEngine::addComponent<LightComponent>(gameObject, col);
    }
};

struct Renderable {
    GameObject *gameObject;
    SpatialComponent *spat;
    RenderableComponent *renderable;

    Renderable(Mesh *mesh, glm::vec3 pos, glm::vec3 scale) {
        gameObject = &NeoEngine::createGameObject();
        spat = &NeoEngine::addComponent<SpatialComponent>(gameObject, pos, scale);
        renderable = &NeoEngine::addComponent<RenderableComponent>(gameObject, mesh);
        renderable->addShaderType<GBufferShader>();
    }
};

struct Decal {
    GameObject *gameObject;
    SpatialComponent *spat;
    RenderableComponent *renderable;

    Decal(Texture2D* texture, glm::vec3 pos, glm::vec3 scale) {
        gameObject = &NeoEngine::createGameObject();
        spat = &NeoEngine::addComponent<SpatialComponent>(gameObject, pos, scale);
        renderable = &NeoEngine::addComponent<RenderableComponent>(gameObject, Loader::getMesh("cube"));
        renderable->addShaderType<DecalShader>();
        NeoEngine::addComponent<DiffuseMapComponent>(gameObject, texture);
        NeoEngine::addComponent<RotationComponent>(gameObject, glm::vec3(0, 1, 0));
    }
};

int main() {
    NeoEngine::init("Deferred", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 20.f);

    std::vector<Light *> lights;
    lights.push_back(new Light(glm::vec3(25.f, 25.f, 0.f), glm::vec3(1.f), glm::vec3(100.f)));

    Renderable cube(Loader::getMesh("cube"), glm::vec3(10.f, 0.75f, 0.f), glm::vec3(5.f));
    NeoEngine::addComponent<MaterialComponent>(cube.gameObject, 0.2f, Util::genRandomVec3());

    Renderable dragon(Loader::getMesh("dragon10k.obj", true), glm::vec3(-4.f, 10.f, -5.f), glm::vec3(10.f));
    NeoEngine::addComponent<MaterialComponent>(dragon.gameObject, 0.2f, Util::genRandomVec3());

    Renderable stairs(Loader::getMesh("staircase.obj", true), glm::vec3(5.f, 10.f, 9.f), glm::vec3(10.f));
    NeoEngine::addComponent<MaterialComponent>(stairs.gameObject, 0.2f, Util::genRandomVec3());

    for (int i = 0; i < 20; i++) {
        Renderable tree(Loader::getMesh("PineTree3.obj", true), glm::vec3(50.f - i * 5.f, 10.f, 25.f + 25.f * Util::genRandom()), glm::vec3(10.f));
        NeoEngine::addComponent<DiffuseMapComponent>(tree.gameObject, Loader::getTexture("PineTexture.png"));
    }

    // Terrain 
    Renderable terrain(Loader::getMesh("quad"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(1000.f));
    terrain.spat->rotate(glm::mat3(glm::rotate(glm::mat4(1.f), -1.56f, glm::vec3(1, 0, 0))));
    NeoEngine::addComponent<MaterialComponent>(terrain.gameObject, 0.7f, glm::vec3(0.7f));
    terrain.renderable->addShaderType<GBufferShader>();

    Decal decal(Loader::getTexture("texture.png"), glm::vec3(0.f, 0.f, -25.f), glm::vec3(15.f));

    /* Systems - order matters! */
    NeoEngine::addSystem<CameraControllerSystem>();
    NeoEngine::addSystem<RotationSystem>();
    NeoEngine::initSystems();

    /* Init renderer */
    MasterRenderer::init("shaders/", camera.camera);
    MasterRenderer::addPreProcessShader<GBufferShader>("gbuffer.vert", "gbuffer.frag");
    MasterRenderer::addPreProcessShader<DecalShader>("decal.vert", "decal.frag"); 
    auto & lightPassShader = MasterRenderer::addPreProcessShader<LightPassShader>("lightpass.vert", "lightpass.frag"); 
    auto & combineShader = MasterRenderer::addPostProcessShader<CombineShader>("combine.frag"); 
    MasterRenderer::addSceneShader<WireframeShader>();

    /* Attach ImGui panes */
    NeoEngine::addDefaultImGuiFunc();

    NeoEngine::addImGuiFunc("Decal", [&]() {
        auto pos = decal.spat->getPosition();
        auto scale = decal.spat->getScale();
        if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -25.f, 25.f)) {
            decal.spat->setPosition(pos);
        }
        if (ImGui::SliderFloat3("Scale", glm::value_ptr(scale), 0.f, 50.f)) {
            decal.spat->setScale(scale);
        }
        static bool showBox = false;
        if (ImGui::Checkbox("Show box", &showBox)) {
            if (showBox) {
                decal.renderable->addShaderType<WireframeShader>();
            }
            else {
                decal.renderable->removeShaderType<WireframeShader>();
            }
        }
    });

    NeoEngine::addImGuiFunc("Lights", [&]() {
        ImGui::Checkbox("Show lights", &lightPassShader.showLights);
        if (lightPassShader.showLights) {
            ImGui::SameLine();
            ImGui::SliderFloat("Show radius", &lightPassShader.showRadius, 0.01f, 1.f);
        }
 
        static int index;
        if (ImGui::CollapsingHeader("Create Lights")) {
            static glm::vec3 pos(0.f);
            static float size(15.f);
            static glm::vec3 color(1.f);
            static float yOffset(10.f);
            ImGui::SliderFloat3("Position", glm::value_ptr(pos), -25.f, 25.f);
            ImGui::SliderFloat("Scale", &size, 15.f, 100.f);
            ImGui::SliderFloat3("Color", glm::value_ptr(color), 0.01f, 1.f);
            ImGui::SliderFloat("Offset", &yOffset, 0.f, 25.f);
            if (ImGui::Button("Create")) {
                auto light = new Light(pos, color, glm::vec3(size));
                lights.push_back(light);
                index = lights.size() - 1;
            }
            ImGui::TreePop();
        }
        if (lights.empty()) {
            return;
        }
        if (ImGui::CollapsingHeader("Edit Lights")) {
            ImGui::SliderInt("Index", &index, 0, lights.size() - 1);
            auto l = lights[index];
            if (ImGui::Button("Delete light")) {
                NeoEngine::removeGameObject(*l->gameObject);
                lights.erase(lights.begin() + index);
                index = glm::max(0, index - 1);
            }
            auto spat = l->gameObject->getSpatial();
            if (!spat) {
                return;
            }
            glm::vec3 pos = spat->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -25.f, 25.f)) {
                spat->setPosition(pos);
            }
            float size = spat->getScale().x;
            if (ImGui::SliderFloat("Scale", &size, 15.f, 100.f)) {
                spat->setScale(glm::vec3(size));
            }
            ImGui::SliderFloat3("Color", glm::value_ptr(l->light->mColor), 0.f, 1.f);
        }
    });

    /* Run */
    NeoEngine::run();
    return 0;
}