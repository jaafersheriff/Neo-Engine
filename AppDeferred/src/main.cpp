#include <NeoEngine.hpp>

#include "CustomSystem.hpp"

#include "GBufferShader.hpp"
#include "LightPassShader.hpp"
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
    Material *material = new Material;

    Renderable(Mesh *mesh, glm::vec3 pos, glm::vec3 scale) {
        gameObject = &NeoEngine::createGameObject();
        spat = &NeoEngine::addComponent<SpatialComponent>(gameObject, pos, scale);
        renderable = &NeoEngine::addComponent<RenderableComponent>(gameObject, mesh);
        NeoEngine::addComponent<MaterialComponent>(gameObject, material);
    }
};

int main() {
    NeoEngine::init("Deferred", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 20.f);
    std::vector<Light *> lights;
    for (int i = 0; i < 50; i++) {
        Renderable r(
            Loader::getMesh("cube"),
            glm::vec3(Util::genRandom(-45.f, 45.f), Util::genRandom(2.f, 5.f), Util::genRandom(-45.f, 45.f)),
            glm::vec3(Util::genRandom(5.f)));
        r.renderable->addShaderType<GBufferShader>();
        r.material->ambient = 0.7f;
        r.material->diffuse = Util::genRandomVec3();
        r.material->specular = Util::genRandomVec3();
    }

    // Terrain 
    Renderable terrain(Loader::getMesh("quad"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(100.f));
    terrain.spat->rotate(glm::mat3(glm::rotate(glm::mat4(1.f), -1.56f, glm::vec3(1, 0, 0))));
    terrain.material->diffuse = glm::vec3(0.7f);
    terrain.material->ambient = 0.7f;
    terrain.renderable->addShaderType<GBufferShader>();

    /* Systems - order matters! */
    NeoEngine::addSystem<CustomSystem>();
    NeoEngine::initSystems();

    /* Init renderer */
    MasterRenderer::init("shaders/", camera.camera);
    MasterRenderer::addPreProcessShader<GBufferShader>("gbuffer.vert", "gbuffer.frag");
    auto & lightPassShader = MasterRenderer::addSceneShader<LightPassShader>("lightpass.vert", "lightpass.frag");

    /* Attach ImGui panes */
    NeoEngine::addImGuiFunc("Stats", [&]() {
        ImGui::Text("FPS: %d", Util::FPS);
        ImGui::Text("dt: %0.4f", Util::timeStep);
        if (ImGui::Button("VSync")) {
            Window::toggleVSync();
        }
    });
    NeoEngine::addImGuiFunc("GBuffer", [&]() {
        auto gbuffer = MasterRenderer::getFBO("gbuffer");
        for (auto texture : gbuffer->textures) {
            ImGui::Image((ImTextureID)texture->textureId, ImVec2(0.1f * texture->width, 0.1f * texture->height), ImVec2(0, 1), ImVec2(1, 0));
        }
    });
    NeoEngine::addImGuiFunc("Lights", [&]() {
        ImGui::Checkbox("Show lights", &lightPassShader.showLights);
        static glm::vec3 pos(0.f);
        static float size(15.f);
        static glm::vec3 color(1.f);
        if (ImGui::Button("Create light")) {
            lights.push_back(new Light(pos, color, glm::vec3(size)));
        }
        if (!lights.size()) {
            return;
        }

        static int index = 0;
        ImGui::SliderInt("Index", &index, 0, lights.size()-1);
        auto l = lights[index];
        if (ImGui::Button("Delete light")) {
            NeoEngine::removeGameObject(*l->gameObject);
            lights.erase(lights.begin() + index);
            if (!lights.size()) {
                return;
            }
        }

        auto spat = l->gameObject->getSpatial();
        if (!spat) {
            return;
        }
        pos = spat->getPosition();
        if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -25.f, 25.f)) {
            spat->setPosition(pos);
        }
        size = spat->getScale().x;
        if (ImGui::SliderFloat("Scale", &size, 15.f, 50.f)) {
            spat->setScale(glm::vec3(size));
        }
        color = l->light->getColor();
        if (ImGui::SliderFloat3("Color", glm::value_ptr(color), 0.f, 1.f)) {
            l->light->setColor(color);
        }
    });

    /* Run */
    NeoEngine::run();
    return 0;
}