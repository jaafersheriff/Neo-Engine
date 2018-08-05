#include <NeoEngine.hpp>

#include "CameraSystem.hpp"

using namespace neo;

/* Systems */
RenderSystem * renderSystem;

/* Game object definitions */
struct Camera {
    GameObject *gameObject;
    CameraControllerComponent *cameraController;
    CameraComponent *cameraComp;

    Camera(float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(*gameObject, pos, glm::vec3(1.f));
        cameraComp = &NeoEngine::addComponent<CameraComponent>(*gameObject, fov, near, far);
        cameraController = &NeoEngine::addComponent<CameraControllerComponent>(*gameObject, ls, ms);
    }
};

struct Skybox {
    GameObject *gameObject;

    Skybox(Texture *tex) {
        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<CubeMapComponent>(*gameObject, tex);
    }
};

int main() {
    NeoEngine::init("Skybox", "res/", 1280, 720);

    /* Game objects */
    Camera camera(45.f, 0.01f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Skybox(Loader::getTexture("nebula_skybox", {"purplenebula_rt.tga", "purplenebula_lf.tga", "purplenebula_up.tga", "purplenebula_dn.tga", "purplenebula_ft.tga", "purplenebula_bk.tga"}, GL_CLAMP_TO_EDGE));

    /* Systems - order matters! */
    NeoEngine::addSystem<CameraSystem>();
    renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/");
    NeoEngine::initSystems();

    /* Attach ImGui panes */
    NeoEngine::addImGuiFunc("Stats", [&]() {
        ImGui::Text("FPS: %d", NeoEngine::FPS);
        ImGui::Text("dt: %0.4f", NeoEngine::timeStep);
        if (ImGui::Button("VSync")) {
            Window::toggleVSync();
        }
    });

    /* Run */
    NeoEngine::run();

    return 0;
}