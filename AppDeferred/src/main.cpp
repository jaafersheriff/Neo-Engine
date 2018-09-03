#include <NeoEngine.hpp>

#include "CustomSystem.hpp"

#include "GBufferShader.hpp"
#include "CombineShader.hpp"
#include "LightPassShader.hpp"
#include "Shader/PhongShader.hpp"

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
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    for (int i = 0; i < 15; i++) {
        Light(
            glm::vec3(Util::genRandom(-45.f, 45.f), Util::genRandom(6.f, 15.f), Util::genRandom(-45.f, 45.f)),
            Util::genRandomVec3(0.f, 1.f),
            glm::vec3(Util::genRandom(25.f, 65.f))
        );
    }

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
    // MasterRenderer::addSceneShader<CombineShader>("combine.vert", "combine.frag");
    MasterRenderer::addSceneShader<LightPassShader>("lightpass.vert", "lightpass.frag");

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

    /* Run */
    NeoEngine::run();
    return 0;
}