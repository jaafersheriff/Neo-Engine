#include <NeoEngine.hpp>

#include "TriangleShader.hpp"

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

/* Custom shader */
TriangleShader * tShader;

int main() {
    /* Init components */
    Camera camera;
    camera.init(45.f, 0.01f, 100.f, glm::vec3(0, 0, -5), glm::vec3(0));

    /* Init systems */
    renderSystem = &NeoEngine::addSystem<RenderSystem>();
    NeoEngine::init("TestApp", "res/", 1280, 720);

    /* Init shaders */
    tShader = &renderSystem->addShader<TriangleShader>("shaders/triangle.vert", "shaders/triangle.frag");

    /* Run */
    NeoEngine::run();

    return 0;
}