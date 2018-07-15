#include <NeoEngine.hpp>

#include "DiffuseShader.hpp"
#include "DiffuseRenderable.hpp"

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
            ImGui::Text("Pos:     %0.2f, %0.2f, %0.2f", position.x, position.y, position.z);
            glm::vec3 lookAt   = cameraComp->getLookAt();
            ImGui::Text("lookAt:  %0.2f, %0.2f, %0.2f", lookAt.x, lookAt.y, lookAt.z);
            glm::vec3 lookDir  = cameraComp->getLookDir();
            ImGui::Text("lookDir: %0.2f, %0.2f, %0.2f", lookDir.x, lookDir.y, lookDir.z);

            glm::mat4 P(cameraComp->getProj());
            glm::mat4 V(cameraComp->getView());
            ImGui::Text("Projection: {%0.2f  %0.2f  %02.f  %0.2f", P[0][0], P[1][0], P[2][0], P[3][0]);
            ImGui::Text("             %0.2f  %0.2f  %02.f  %0.2f", P[0][1], P[1][1], P[2][1], P[3][1]); 
            ImGui::Text("             %0.2f  %0.2f  %02.f  %0.2f", P[0][2], P[1][2], P[2][2], P[3][2]); 
            ImGui::Text("             %0.2f  %0.2f  %02.f  %0.2f", P[0][3], P[1][3], P[2][3], P[3][3]); 
            ImGui::Text("View:       {%0.2f  %0.2f  %02.f  %0.2f", V[0][0], V[1][0], V[2][0], V[3][0]);
            ImGui::Text("             %0.2f  %0.2f  %02.f  %0.2f", V[0][1], V[1][1], V[2][1], V[3][1]); 
            ImGui::Text("             %0.2f  %0.2f  %02.f  %0.2f", V[0][2], V[1][2], V[2][2], V[3][2]); 
            ImGui::Text("             %0.2f  %0.2f  %02.f  %0.2f", V[0][3], V[1][3], V[2][3], V[3][3]); 
         
            ImGui::End();
        });
    }
};

/* Cube */
struct Cube {
    GameObject *gameObject;
    DiffuseRenderable *diffComponent;

    void init(std::string name, glm::vec3 p, float s, glm::vec3 r) {
        Mesh *mesh = Loader::getMesh(name);

        gameObject = &NeoEngine::createGameObject();
        diffComponent = &NeoEngine::addComponent<DiffuseRenderable>(*gameObject, mesh, p, s, r);

        NeoEngine::addImGuiFunc([&]() {
            ImGui::Begin("Cube");
            ImGui::SliderFloat3("Position", glm::value_ptr(diffComponent->position), -10.f, 10.f);
            ImGui::SliderFloat("Scale", &diffComponent->scale, 0.f, 10.f);
            ImGui::SliderFloat3("Rotation", glm::value_ptr(diffComponent->rotation), 0.f, 4.f);
            glm::mat4 M(diffComponent->M);
            ImGui::Text("Mrojection: {%0.2f  %0.2f  %02.f  %0.2f", M[0][0], M[1][0], M[2][0], M[3][0]);
            ImGui::Text("             %0.2f  %0.2f  %02.f  %0.2f", M[0][1], M[1][1], M[2][1], M[3][1]); 
            ImGui::Text("             %0.2f  %0.2f  %02.f  %0.2f", M[0][2], M[1][2], M[2][2], M[3][2]); 
            ImGui::Text("             %0.2f  %0.2f  %02.f  %0.2f", M[0][3], M[1][3], M[2][3], M[3][3]); 
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

    /* Init components */
    Camera camera;
    camera.init(45.f, 0.01f, 100.f, glm::vec3(0, 0, -5), glm::vec3(0));
    Cube cube;
    cube.init("cube.obj", glm::vec3(0.f), 10.f, glm::vec3(0.f));
    
    /* Init systems */
    renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/", camera.cameraComp);
    NeoEngine::initSystems();

    /* Attach shaders */
    dShader = &renderSystem->addShader<DiffuseShader>("diffuse.vert", "diffuse.frag");

    /* Run */
    NeoEngine::run();

    return 0;
}