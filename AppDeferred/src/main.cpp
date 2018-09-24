#include <NeoEngine.hpp>

#include "CustomSystem.hpp"

#include "SinMoveComponent.hpp"

#include "Shader/WireframeShader.hpp"
#include "GBufferShader.hpp"
#include "LightPassShader.hpp"
#include "AOShader.hpp"
#include "CombineShader.hpp"
#include "BlurShader.hpp"

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

int main() {
    NeoEngine::init("Deferred", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 20.f);

    std::vector<Light *> lights;
    lights.push_back(new Light(glm::vec3(25.f, 25.f, 0.f), glm::vec3(1.f), glm::vec3(100.f)));
    Renderable cube(Loader::getMesh("cube"), glm::vec3(10.f, 0.75f, 0.f), glm::vec3(5.f));
    NeoEngine::addComponent<MaterialComponent>(cube.gameObject, Loader::getMaterial("cube mat", 0.2f, Util::genRandomVec3()));
    Renderable dragon(Loader::getMesh("dragon10k.obj", true), glm::vec3(-4.f, 10.f, -5.f), glm::vec3(10.f));
    NeoEngine::addComponent<MaterialComponent>(dragon.gameObject, Loader::getMaterial("dragon mat", 0.2f, Util::genRandomVec3()));
    Renderable stairs(Loader::getMesh("staircase.obj", true), glm::vec3(5.f, 10.f, 9.f), glm::vec3(10.f));
    NeoEngine::addComponent<MaterialComponent>(stairs.gameObject, Loader::getMaterial("stairs mat", 0.2f, Util::genRandomVec3()));
    for (int i = 0; i < 20; i++) {
        Renderable tree(Loader::getMesh("PineTree3.obj", true), glm::vec3(50.f - i * 5.f, 10.f, 25.f + 25.f * Util::genRandom()), glm::vec3(10.f));
        NeoEngine::addComponent<DiffuseMapComponent>(tree.gameObject, Loader::getTexture("PineTexture.png"));
    }

    // Terrain 
    Renderable terrain(Loader::getMesh("quad"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(1000.f));
    terrain.spat->rotate(glm::mat3(glm::rotate(glm::mat4(1.f), -1.56f, glm::vec3(1, 0, 0))));
    NeoEngine::addComponent<MaterialComponent>(terrain.gameObject, Loader::getMaterial("terrain mat", 0.7f, glm::vec3(0.7f)));
    terrain.renderable->addShaderType<GBufferShader>();

    /* Systems - order matters! */
    NeoEngine::addSystem<CustomSystem>();
    NeoEngine::initSystems();

    /* Init renderer */
    MasterRenderer::init("shaders/", camera.camera);
    MasterRenderer::addPreProcessShader<GBufferShader>("gbuffer.vert", "gbuffer.frag");
    auto & lightPassShader = MasterRenderer::addPreProcessShader<LightPassShader>("lightpass.vert", "lightpass.frag");  // run light pass after generating gbuffer
    auto & aoShader = MasterRenderer::addPostProcessShader<AOShader>("ao.frag");    // first post process - generate ssao map 
    auto & blurShader = MasterRenderer::addPostProcessShader<BlurShader>("blur.frag"); // blur ssao map
    MasterRenderer::addPostProcessShader<CombineShader>("combine.frag");    // combine light pass and ssao 

    /* Attach ImGui panes */
    NeoEngine::addDefaultImGuiFunc();
    NeoEngine::addImGuiFunc("AO", [&]() {
        int size = Loader::getTexture("aoKernel")->width;
        if (ImGui::SliderInt("Kernel", &size, 1, 128)) {
            aoShader.generateKernel(size);
        }
        size = Loader::getTexture("aoNoise")->width;
        if (ImGui::SliderInt("Noise", &size, 1, 32)) {
            aoShader.generateNoise(size);
        }
        ImGui::SliderFloat("Radius", &aoShader.radius, 0.f, 1.f);
        ImGui::SliderFloat("Bias", &aoShader.bias, 0.f, 1.f);
        ImGui::SliderInt("Blur", &blurShader.blurAmount, 0, 10);
    });
    NeoEngine::addImGuiFunc("Lights", [&]() {
        ImGui::Checkbox("Show lights", &lightPassShader.showLights);
        if (lightPassShader.showLights) {
            ImGui::SameLine();
            ImGui::SliderFloat("Show radius", &lightPassShader.showRadius, 0.01f, 1.f);
        }
 
        static int index;
        if (ImGui::CollapsingHeader("Create Lights")) {
            if (ImGui::TreeNode("Single")) {
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
                    if (yOffset) {
                        NeoEngine::addComponent<SinMoveComponent>(light->gameObject, yOffset, pos.y);
                    }
                    lights.push_back(light);
                    index = lights.size() - 1;
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Random Lights")) {
                if (ImGui::Button("Clear lights")) {
                    for (auto & l : lights) {
                        NeoEngine::removeGameObject(*l->gameObject);
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
                        NeoEngine::addComponent<SinMoveComponent>(light->gameObject, Util::genRandom(minSinOffset, maxSinOffset), position.y);
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
            glm::vec3 color = l->light->getColor();
            if (ImGui::SliderFloat3("Color", glm::value_ptr(color), 0.f, 1.f)) {
                l->light->setColor(color);

            }
        }
    });

    /* Run */
    NeoEngine::run();
    return 0;
}