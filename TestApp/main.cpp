#include <NeoEngine.hpp>

#include "CustomShader.hpp"
#include "WireShader.hpp"
#include "CustomComponent.hpp"
#include "CustomSystem.hpp"

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

        NeoEngine::addImGuiFunc("Camera", [&]() {
            float fov = cameraComp->getFOV();
            ImGui::SliderFloat("FOV", &fov, 0.f, 90.f);
            cameraComp->setFOV(fov);

            float near = cameraComp->getNear();
            float far = cameraComp->getFar();
            ImGui::SliderFloat("Near", &near, 0.f, 2.f);
            ImGui::SliderFloat("Far", &far, 10.f, 10000.f);
            cameraComp->setNearFar(near, far);

            glm::vec3 position = gameObject->getSpatial()->getPosition();
            ImGui::Text("Pos:     %0.2f, %0.2f, %0.2f", position.x, position.y, position.z);
            glm::vec3 lookDir  = cameraComp->getLookDir();
            ImGui::Text("lookDir: %0.2f, %0.2f, %0.2f", lookDir.x, lookDir.y, lookDir.z);
        });
    }
};

struct Renderable {
    GameObject *gameObject;
    RenderableComponent *renderComp;

    Renderable(Mesh *m, glm::vec3 p, float s, glm::mat3 o = glm::mat3()) {
        Mesh *mesh = m;

        gameObject = &NeoEngine::createGameObject();
        NeoEngine::addComponent<SpatialComponent>(*gameObject, p, glm::vec3(s), o);
        renderComp = &NeoEngine::addComponent<RenderableComponent>(*gameObject, mesh);
    }

    bool customShaderEnabled = true;
    bool wireShaderEnabled = true;
    void attachImGui(const std::string & name) {
        NeoEngine::addImGuiFunc(name, [&]() {
            ImGui::Text("Components: %d", gameObject->getNumberComponents());
            if (ImGui::Button("Add custom component")) {
                NeoEngine::addComponent<CustomComponent>(*gameObject);
            }
            if (ImGui::Button("Remove custom component")) {
                NeoEngine::removeComponent<CustomComponent>(*gameObject->getComponentByType<CustomComponent>());
            }
 
            ImGui::Text("Shaders: %d", renderComp->getShaders().size());
            if (ImGui::Button("Custom Shader")) {
                customShaderEnabled = !customShaderEnabled;
                if (customShaderEnabled) {
                    renderSystem->attachShaderToComp<CustomShader>(renderComp);
                }
                else {
                    renderSystem->detachShaderFromComp<CustomShader>(renderComp);
                }
            }
            if (ImGui::Button("Wire Shader")) {
                wireShaderEnabled = !wireShaderEnabled;
                if (wireShaderEnabled) {
                    renderSystem->attachShaderToComp<WireShader>(renderComp);
                }
                else {
                    renderSystem->detachShaderFromComp<WireShader>(renderComp);
                }
            }
            glm::vec3 pos = gameObject->getSpatial()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -10.f, 10.f)) {
                gameObject->getSpatial()->setPosition(pos);
            }
            float scale = gameObject->getSpatial()->getScale().x;
            if (ImGui::SliderFloat("Scale", &scale, 0.f, 10.f)) {
                gameObject->getSpatial()->setScale(glm::vec3(scale));
            }
        });
    }
};

int main() {
    NeoEngine::init("TestApp", "res/", 1280, 720);

    /* Init engine-necessary components */
    Camera camera(45.f, 0.01f, 100.f, glm::vec3(0, 0.6f, 5), 2.f, 5.f);
   
    /* Systems - order matters! */
    NeoEngine::addSystem<CustomSystem>(camera.cameraController);
    renderSystem = &NeoEngine::addSystem<RenderSystem>("shaders/", camera.cameraComp);
    NeoEngine::initSystems();

    /* Shaders */
    renderSystem->addShader<CustomShader>("custom.vert", "custom.frag");
    renderSystem->addShader<WireShader>("wire.vert", "wire.frag");

    /* Init app components */
    Renderable cube(Loader::getMesh("cube.obj"), glm::vec3(0.f), 1.f, glm::mat3(glm::rotate(glm::mat4(1.f), 0.707f, glm::vec3(1, 0, 0))));
    renderSystem->attachShaderToComp<CustomShader>(cube.renderComp);
    renderSystem->attachShaderToComp<WireShader>(cube.renderComp);

    /* Attach ImGui panes */
    cube.attachImGui("Cube");
    NeoEngine::addImGuiFunc("Stats", [&]() {
        ImGui::Text("FPS: %d", NeoEngine::FPS);
        ImGui::Text("dt: %0.4f", NeoEngine::timeStep);
        if (ImGui::Button("VSync")) {
            Window::toggleVSync();
        }
    });
    NeoEngine::addImGuiFunc("Mouse", [&]() {
        ImGui::Text("Mouse X, Y  : %0.2f, %0.2f", Mouse::x, Mouse::y);
        ImGui::Text("Mouse dx, dy: %0.2f, %0.2f", Mouse::dx, Mouse::dy);
    });
    NeoEngine::addImGuiFunc("Engine", [&]() {
        ImGui::Text("GameObjects:  %d", NeoEngine::getGameObjects().size());
        int count = 0;
        for (auto go : NeoEngine::getGameObjects()) {
            count += go->getNumberComponents();
        }
        ImGui::Text("Components:  %d", count);
        ImGui::Text("Systems: %d", NeoEngine::getSystems().size());
    });
    NeoEngine::addImGuiFunc("Systems", [&]() {
        for (auto sys : NeoEngine::getSystems()) {
            ImGui::Checkbox(sys.second->name.c_str(), &sys.second->active);
        }
    });
    NeoEngine::addImGuiFunc("Render System", [&]() {
        ImGui::Text("Shaders:  %d", renderSystem->shaders.size());
        for (auto it(renderSystem->shaders.begin()); it != renderSystem->shaders.end(); ++it) {
            ImGui::Checkbox(it->get()->name().c_str(), &it->get()->active);
        }

        int size = 0;
        for (auto it(renderSystem->renderables.begin()); it != renderSystem->renderables.end(); ++it) {
            size += it->second->size();
        }
        ImGui::Text("Renderables: %d", size);
    });

    /* Run */
    NeoEngine::run();

    return 0;
}
