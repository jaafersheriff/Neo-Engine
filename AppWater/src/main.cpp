#include <Engine.hpp>

#include "GBufferShader.hpp"
#include "GBufferSkyboxShader.hpp"
#include "LightPassShader.hpp"
#include "CombineShader.hpp"
#include "Shader/WireframeShader.hpp"
#include "Shader/GammaCorrectShader.hpp"

#include "WaterComponent.hpp"
#include "WaterMeshComponent.hpp"
#include "WaterShader.hpp"

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
    config.APP_NAME = "Water";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 20.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    /* Lights */
    std::vector<Light *> lights;
    lights.push_back(new Light(glm::vec3(25.f, 25.f, 0.f), glm::vec3(1.f), glm::vec3(100.f)));

    /* Renderables */
    Renderable cube(Library::getMesh("cube"), glm::vec3(10.f, 0.75f, 0.f), glm::vec3(5.f));
    Engine::addComponent<MaterialComponent>(cube.gameObject, 0.2f, Util::genRandomVec3());
    Renderable dragon(Library::getMesh("dragon10k.obj", true), glm::vec3(4.f, 5.f, 5.f), glm::vec3(10.f));
    Engine::addComponent<MaterialComponent>(dragon.gameObject, 0.2f, Util::genRandomVec3());

    /* Skybox */
    {
        GameObject* gameObject = &Engine::createGameObject();
        Engine::addComponent<renderable::SkyboxComponent>(gameObject);
        Engine::addComponent<CubeMapComponent>(gameObject, *Library::getCubemap("arctic_skybox", {"arctic_ft.tga", "arctic_bk.tga", "arctic_up.tga", "arctic_dn.tga", "arctic_rt.tga", "arctic_lf.tga"}));
    }



    /* Water */
    {
        auto& go = Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(&go, glm::vec3(-20.f, 5.f, -15.f));
        Engine::addComponent<WaterMeshComponent>(&go, 50, 50, 5.f, 5.f);
        Engine::addComponent<WaterComponent>(&go);
    }

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<SinTranslateSystem>();

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addPreProcessShader<GBufferShader>("gbuffer.vert", "gbuffer.frag");
    Renderer::addPreProcessShader<GBufferSkyboxShader>("gbufferskybox.vert", "gbufferskybox.frag");
    Renderer::addPreProcessShader<LightPassShader>("lightpass.vert", "lightpass.frag"); 
    Renderer::addSceneShader<WaterShader>("water.vert", "water.frag", "water.control", "water.eval");
    Renderer::addPostProcessShader<CombineShader>("combine.frag"); 
    Renderer::addPostProcessShader<GammaCorrectShader>();

    /* Attach ImGui panes */
    Engine::addImGuiFunc("Water", [&]() {
        if (auto water = Engine::getComponentTuple<WaterComponent, WaterMeshComponent, SpatialComponent>()) {
            water->get<WaterComponent>()->imGuiEditor();
            water->get<WaterMeshComponent>()->imGuiEditor();
            water->get<SpatialComponent>()->imGuiEditor();
        }
    });
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