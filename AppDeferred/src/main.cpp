#include <NeoEngine.hpp>

#include "CustomSystem.hpp"

#include "GBufferShader.hpp"
#include "LightShader.hpp"
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

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(gameObject, pos);
        light = &NeoEngine::addComponent<LightComponent>(gameObject, col, att);

        NeoEngine::addImGuiFunc("Light", [&]() {
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -100.f, 100.f)) {
                gameObject->getSpatial()->setPosition(pos);
            }
            glm::vec3 col = light->getColor();
            if (ImGui::SliderFloat3("Color", glm::value_ptr(col), 0.f, 1.f)) {
                light->setColor(col);
            }
            glm::vec3 att = light->getAttenuation();
            ImGui::SliderFloat3("Attenuation", glm::value_ptr(att), 0.f, 1.f);
            light->setAttenuation(att);
        });
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

        NeoEngine::addImGuiFunc("Mesh", [&]() {
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -10.f, 10.f)) {
                gameObject->getSpatial()->setPosition(pos);
            }
            float scale = gameObject->getSpatial()->getScale().x;
            if (ImGui::SliderFloat("Scale", &scale, 0.f, 10.f)) {
                gameObject->getSpatial()->setScale(glm::vec3(scale));
            }
            static glm::vec3 rot(0.f);
            if (ImGui::SliderFloat3("Rotation", glm::value_ptr(rot), 0.f, 4.f)) {
                glm::mat4 R;
                R = glm::rotate(glm::mat4(1.f), rot.x, glm::vec3(1, 0, 0));
                R *= glm::rotate(glm::mat4(1.f), rot.y, glm::vec3(0, 1, 0));
                R *= glm::rotate(glm::mat4(1.f), rot.z, glm::vec3(0, 0, 1));
                gameObject->getSpatial()->setOrientation(glm::mat3(R));
            }
        });
    }
};

int main() {
    NeoEngine::init("Deferred", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    for (int i = 0; i < 50; i++) {
        Renderable r(
            Util::genRandomBool() ? Loader::getMesh("cube") : Loader::getMesh("sphere"),
            glm::vec3(Util::genRandom(-45.f, 45.f), Util::genRandom(2.f, 5.f), Util::genRandom(-45.f, 45.f)),
            glm::vec3(Util::genRandom(5.f)));
        r.renderable->addShaderType<GBufferShader>();
        r.material->ambient = 0.5f;
        r.material->diffuse = Util::genRandomVec3();
    }

    // Terrain 
    Renderable terrain(Loader::getMesh("quad"), glm::vec3(0.f, 0.f, 0.f), glm::vec3(100.f));
    terrain.spat->rotate(glm::mat3(glm::rotate(glm::mat4(1.f), -1.56f, glm::vec3(1, 0, 0))));
    terrain.material->diffuse = glm::vec3(0.7f);
    terrain.renderable->addShaderType<GBufferShader>();

    /* Systems - order matters! */
    NeoEngine::addSystem<CustomSystem>();
    NeoEngine::initSystems();

    /* Init renderer */
    MasterRenderer::init("shaders/", camera.camera);
    MasterRenderer::addPreProcessShader<GBufferShader>("gbuffer.vert", "gbuffer.frag");
    MasterRenderer::addSceneShader<LightShader>("lightpass.vert", "lightpass.frag");

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