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
    NeoEngine::attachGuiFunc([&]() {
        ImGui::Begin("Camera");
        float fov = camera.cameraComp->getFOV();
        ImGui::SliderFloat("FOV", &fov, 0.f, 90.f);
        camera.cameraComp->setFOV(fov);
        ImGui::Text("%0.2f", camera.cameraComp->getFOV());

        float near = camera.cameraComp->getNear();
        ImGui::SliderFloat("Near", &near, 0.f, 2.f);
        float far = camera.cameraComp->getFar();
        ImGui::SliderFloat("Far", &far, 10.f, 10000.f);
        camera.cameraComp->setNearFar(near, far);
        ImGui::Text("%0.2f", camera.cameraComp->getNear());
        ImGui::Text("%0.2f", camera.cameraComp->getFar());

        glm::vec3 position = camera.cameraComp->getPosition();
        ImGui::Text("%0.2f, %0.2f, %0.2f", position.x, position.y, position.z);
        glm::vec3 lookAt   = camera.cameraComp->getLookAt();
        ImGui::Text("%0.2f, %0.2f, %0.2f", lookAt.x, lookAt.y, lookAt.z);
        glm::vec3 lookDir  = camera.cameraComp->getLookDir();
        ImGui::Text("%0.2f, %0.2f, %0.2f", lookDir.x, lookDir.y, lookDir.z);
        ImGui::End();
    });

    /* Init systems */
    renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/");
    NeoEngine::init("TestApp", "res/", 1280, 720);

    /* Attach shaders */
    tShader = &renderSystem->addShader<TriangleShader>("triangle.vert", "triangle.frag");

    /* Run */
    NeoEngine::run();

    return 0;
}