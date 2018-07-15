#include <NeoEngine.hpp>

#include "DiffuseShader.hpp"

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
        NeoEngine::addImGuiFunc([&]() {
            ImGui::Begin("Camera");
            float fov = cameraComp->getFOV();
            ImGui::SliderFloat("FOV", &fov, 0.f, 90.f);
            cameraComp->setFOV(fov);

            float near = cameraComp->getNear();
            ImGui::SliderFloat("Near", &near, 0.f, 2.f);
            float far = cameraComp->getFar();
            ImGui::SliderFloat("Far", &far, 10.f, 10000.f);
            cameraComp->setNearFar(near, far);

            glm::vec3 position = cameraComp->getPosition();
            ImGui::Text("%0.2f, %0.2f, %0.2f", position.x, position.y, position.z);
            glm::vec3 lookAt   = cameraComp->getLookAt();
            ImGui::Text("%0.2f, %0.2f, %0.2f", lookAt.x, lookAt.y, lookAt.z);
            glm::vec3 lookDir  = cameraComp->getLookDir();
            ImGui::Text("%0.2f, %0.2f, %0.2f", lookDir.x, lookDir.y, lookDir.z);
            ImGui::End();
        });
    }
};

/* Custom shader */
DiffuseShader * dShader;

int main() {
    NeoEngine::init("TestApp", "res/", 1280, 720);
    NeoEngine::addImGuiFunc([&]() {
        ImGui::Begin("Stats");
        ImGui::Text("FPS: %d", NeoEngine::FPS);
        ImGui::Text("dt: %0.5f", NeoEngine::timeStep);
        if (ImGui::Button("VSync")) {
            Window::toggleVSync();
        }
        ImGui::End();
    });
    
    /* Init systems */
    renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/");
    NeoEngine::initSystems();

    /* Attach shaders */
    dShader = &renderSystem->addShader<DiffuseShader>("diffuse.vert", "diffuse.frag");

    /* Init components */
    Camera camera;
    camera.init(45.f, 0.01f, 100.f, glm::vec3(0, 0, -5), glm::vec3(0));

    /* Run */
    NeoEngine::run();

    return 0;
}