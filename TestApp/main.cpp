#include <NeoEngine.hpp>

#include "CustomShader.hpp"
#include "CustomRenderable.hpp"

using namespace neo;

/* Common system */
RenderSystem * renderSystem;

/* Common game objects */
struct Camera {
    GameObject *gameObject;
    CameraComponent *cameraComp;

    Camera(float fov, float near, float far, glm::vec3 pos, glm::vec3 lookAt) {
        gameObject = &NeoEngine::createGameObject();
        cameraComp = &NeoEngine::addComponent<CameraComponent>(*gameObject, fov, near, far, pos, lookAt);

        NeoEngine::addImGuiFunc([&]() {
            ImGui::Begin("Camera");

            float fov = cameraComp->getFOV();
            ImGui::SliderFloat("FOV", &fov, 0.f, 90.f);
            cameraComp->setFOV(fov);

            float near = cameraComp->getNear();
            float far = cameraComp->getFar();
            ImGui::SliderFloat("Near", &near, 0.f, 2.f);
            ImGui::SliderFloat("Far", &far, 10.f, 10000.f);
            cameraComp->setNearFar(near, far);

            glm::vec3 position = cameraComp->getPosition();
            ImGui::Text("Pos:     %0.2f, %0.2f, %0.2f", position.x, position.y, position.z);
            glm::vec3 lookAt   = cameraComp->getLookAt();
            ImGui::Text("lookAt:  %0.2f, %0.2f, %0.2f", lookAt.x, lookAt.y, lookAt.z);
            glm::vec3 lookDir  = cameraComp->getLookDir();
            ImGui::Text("lookDir: %0.2f, %0.2f, %0.2f", lookDir.x, lookDir.y, lookDir.z);
            ImGui::End();
        });
    }
};

/* Basic renderable object */
struct Renderable {
    GameObject *gameObject;
    CustomRenderable *renderComponent;

    Renderable(std::string name, glm::vec3 p, float s, glm::vec3 r) {
        Mesh *mesh = Loader::getMesh(name);

        gameObject = &NeoEngine::createGameObject();
        renderComponent = &NeoEngine::addComponent<CustomRenderable>(*gameObject, mesh, p, s, r);
    }

    void attachImGui(const std::string & name) {
        NeoEngine::addImGuiFunc([&]() {
            ImGui::Begin(name.c_str());
            ImGui::SliderFloat3("Position", glm::value_ptr(renderComponent->position), -10.f, 10.f);
            ImGui::SliderFloat("Scale", &renderComponent->scale, 0.f, 10.f);
            ImGui::SliderFloat3("Rotation", glm::value_ptr(renderComponent->rotation), -4.f, 4.f);
            ImGui::End();
        });
    }
};

/* Custom shader */
CustomShader *dShader;

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

    /* Init components */
    Camera camera(45.f, 0.01f, 100.f, glm::vec3(0, 0, -5), glm::vec3(0));
    Renderable cube("cube.obj", glm::vec3(0.f), 1.f, glm::vec3(-0.707f, 0.f, 0.f));
    cube.attachImGui("Cube");
    
    /* Init systems */
    renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/", camera.cameraComp);
    NeoEngine::initSystems();

    /* Attach shaders */
    dShader = &renderSystem->addShader<CustomShader>("custom.vert", "custom.frag");

    /* Run */
    NeoEngine::run();

    return 0;
}
