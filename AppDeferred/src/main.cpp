#include <Engine.hpp>

#include "GBufferShader.hpp"
#include "LightPassShader.hpp"
#include "CombineShader.hpp"
#include "Shader/WireframeShader.hpp"
#include "Shader/GammaCorrectShader.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "Util/Util.hpp"

using namespace neo;

/* Game object definitions */
struct Camera {
    CameraComponent *camera;
    Camera(float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        GameObject *gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov, Window::getAspectRatio());
        Engine::addComponent<CameraControllerComponent>(gameObject, ls, ms);
    }
};

struct Light {
    GameObject *gameObject;
    LightComponent *light;

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 scale) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, scale);
        light = &Engine::addComponent<LightComponent>(gameObject, col);
    }
};

struct Renderable {
    GameObject *gameObject;
    SpatialComponent *spat;

    Renderable(Mesh *mesh, glm::vec3 pos, glm::vec3 scale) {
        gameObject = &Engine::createGameObject();
        spat = &Engine::addComponent<SpatialComponent>(gameObject, pos, scale);
        Engine::addComponent<MeshComponent>(gameObject, mesh);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Deferred";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 20.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    std::vector<Light *> lights;
    lights.push_back(new Light(glm::vec3(25.f, 25.f, 0.f), glm::vec3(1.f), glm::vec3(100.f)));
    Renderable cube(Library::getMesh("cube"), glm::vec3(10.f, 0.75f, 0.f), glm::vec3(5.f));
    Engine::addComponent<MaterialComponent>(cube.gameObject, 0.2f, Util::genRandomVec3());
    Renderable dragon(Library::getMesh("dragon10k.obj", true), glm::vec3(-4.f, 10.f, -5.f), glm::vec3(10.f));
    Engine::addComponent<MaterialComponent>(dragon.gameObject, 0.2f, Util::genRandomVec3());
    Renderable stairs(Library::getMesh("staircase.obj", true), glm::vec3(5.f, 10.f, 9.f), glm::vec3(10.f));
    Engine::addComponent<MaterialComponent>(stairs.gameObject, 0.2f, Util::genRandomVec3());
    for (int i = 0; i < 20; i++) {
        Renderable tree(Library::getMesh("PineTree3.obj", true), glm::vec3(50.f - i * 5.f, 10.f, 25.f + 25.f * Util::genRandom()), glm::vec3(10.f));
        Engine::addComponent<DiffuseMapComponent>(tree.gameObject, *Library::getTexture("PineTexture.png"));
    }

    // Terrain 
    // TODO : used hilly plain -- add perlin to meshgen
    Renderable terrain(Library::getMesh("quad"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(1000.f));
    terrain.spat->rotate(glm::mat3(glm::rotate(glm::mat4(1.f), -1.56f, glm::vec3(1, 0, 0))));
    Engine::addComponent<MaterialComponent>(terrain.gameObject, 0.7f, glm::vec3(0.7f));

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<SinTranslateSystem>();

    /* Init renderer */
    Renderer::init("shaders/", camera.camera);
    Renderer::addPreProcessShader<GBufferShader>("gbuffer.vert", "gbuffer.frag");
    Renderer::addPreProcessShader<LightPassShader>("lightpass.vert", "lightpass.frag"); 
    Renderer::addPostProcessShader<CombineShader>("combine.frag"); 
    Renderer::addPostProcessShader<GammaCorrectShader>();

    /* Attach ImGui panes */
    Engine::addImGuiFunc("Lights", [&]() {
        static int index;
        if (ImGui::CollapsingHeader("Create Lights")) {
            if (ImGui::TreeNode("Single")) {
                static glm::vec3 pos(0.f);
                static float size(15.f);
                static glm::vec3 color(1.f);
                static glm::vec3 yOffset(0.f, 10.f, 0.f);
                ImGui::SliderFloat3("Position", glm::value_ptr(pos), -25.f, 25.f);
                ImGui::SliderFloat("Scale", &size, 15.f, 100.f);
                ImGui::SliderFloat3("Color", glm::value_ptr(color), 0.01f, 1.f);
                ImGui::SliderFloat("Offset", &yOffset.y, 0.f, 25.f);
                if (ImGui::Button("Create")) {
                    auto light = new Light(pos, color, glm::vec3(size));
                    if (yOffset.y) {
                        Engine::addComponent<SinTranslateComponent>(light->gameObject, yOffset, pos);
                    }
                    lights.push_back(light);
                    index = lights.size() - 1;
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Random Lights")) {
                if (ImGui::Button("Clear lights")) {
                    for (auto & l : lights) {
                        Engine::removeGameObject(*l->gameObject);
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
                ImGui::SliderFloat3("Min offset", glm::value_ptr(minOffset), -100.f, 100.f);
                ImGui::SliderFloat3("Max offset", glm::value_ptr(maxOffset), -100.f, 100.f);
                ImGui::SliderFloat("Min scale", &minScale, 0.f, maxScale);
                ImGui::SliderFloat("Max scale", &maxScale, minScale, 100.f);
                ImGui::SliderFloat("Min sin", &minSinOffset, 0.f, 15.f);
                ImGui::SliderFloat("Max sin", &maxSinOffset, 0.f, 15.f);
                if (ImGui::Button("Create light")) {
                    for (int i = 0; i < numLights; i++) {
                        glm::vec3 position = glm::vec3(
                            Util::genRandom(minOffset.x, maxOffset.x),
                            Util::genRandom(minOffset.y, maxOffset.y),
                            Util::genRandom(minOffset.z, maxOffset.z)
                        );
                        glm::vec3 color = Util::genRandomVec3();
                        float size = Util::genRandom(minScale, maxScale);
                        auto light = new Light(position, color, glm::vec3(size));
                        glm::vec3 sinMove(0.f, Util::genRandom(minSinOffset, maxSinOffset), 0.f);
                        Engine::addComponent<SinTranslateComponent>(light->gameObject, sinMove, position);
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
            ImGui::SliderInt("Index", &index, 0, lights.size() - 1);
            auto l = lights[index];
            if (ImGui::Button("Delete light")) {
                Engine::removeGameObject(*l->gameObject);
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