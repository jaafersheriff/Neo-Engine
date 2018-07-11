#include <NeoEngine.hpp>

using namespace neo;

/* Common system */
RenderSystem * renderSystem;

/* Common game objects */
struct Camera {
    GameObject *gameObject;
    CameraComponent * cameraComp;

    void init(float fov, float near, float far, glm::vec3 pos, glm::vec3 lookAt) {
        gameObject = &NeoEngine::createGameObject();
        cameraComp = &NeoEngine::addComponent<CameraComponent>(*gameObject, fov, near, far, pos, lookAt);
    }
};

int main() {
    /* Create camera */
    Camera camera;
    camera.init(45.f, 0.01f, 100.f, glm::vec3(0, 0, -5), glm::vec3(0));

    /* Init render system */
    renderSystem = &NeoEngine::addSystem<RenderSystem>();

    /* Run */
    NeoEngine::init("TestApp", "", 1280, 720);
    NeoEngine::run();

    return 0;
}