#include <Engine.hpp>

#include "Shader/PhongShader.hpp"
#include "Shader/AlphaTestShader.hpp"
#include "Shader/GammaCorrectShader.hpp"
#include "Loader/MeshGenerator.hpp"

#include "DofBlurShader.hpp"
#include "DofInfoShader.hpp"
#include "DofDownShader.hpp"
#include "PostShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
struct Camera {
    CameraComponent *camera;
    Camera(float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        GameObject *gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov, Window::getAspectRatio());
        Engine::addComponent<CameraControllerComponent>(gameObject, ls, ms);
    }
};

struct Light {
    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        auto& gameObject = Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(&gameObject, pos);
        Engine::addComponent<LightComponent>(&gameObject, col, att);

        Engine::addImGuiFunc("Light", [&]() {
            auto light = Engine::getSingleComponent<LightComponent>();
            light->imGuiEditor();
            if (auto spatial = light->getGameObject().getComponentByType<SpatialComponent>()) {
                spatial->imGuiEditor();
            }
        });
    }
};

struct Renderable {
    GameObject *gameObject;

    Renderable(Mesh *mesh, glm::vec3 position = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f), glm::vec3 rotation = glm::vec3(0.f)) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<MeshComponent>(gameObject, mesh);
        Engine::addComponent<SpatialComponent>(gameObject, position, scale, rotation);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Base";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

    /* Scene objects */
    for (int i = 0; i < 50; i++) {
        auto mesh = i % 2 ? Library::getMesh("cube") : Library::getMesh("sphere");
        Renderable renderable(mesh, glm::vec3(Util::genRandom(-30.f, 30.f), 1.f, Util::genRandom(-30.f, 30.f)), glm::vec3(Util::genRandom(1.5f, 4.5f)), Util::genRandomVec3(-Util::PI, Util::PI));
        Engine::addComponent<renderable::PhongRenderable>(renderable.gameObject);
        Engine::addComponent<MaterialComponent>(renderable.gameObject, 0.2f, Util::genRandomVec3(0.2f, 1.f), glm::vec3(1.f));
        Engine::addComponent<SelectableComponent>(renderable.gameObject);
        Engine::addComponent<BoundingBoxComponent>(renderable.gameObject, mesh);
    }

    /* Ground plane */
    Renderable plane(Library::getMesh("quad"), glm::vec3(0.f), glm::vec3(100.f), glm::vec3(-Util::PI / 2.f, 0.f, 0.f));
    Engine::addComponent<renderable::AlphaTestRenderable>(plane.gameObject);
    Engine::addComponent<DiffuseMapComponent>(plane.gameObject, *Library::getTexture("grid.png"));

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();

    /* Init renderer */
    auto defaultFBO = Library::getFBO("default");
    defaultFBO->attachColorTexture(Window::getFrameSize(), { GL_RGBA, GL_RGBA, GL_NEAREST, GL_CLAMP_TO_EDGE });
    defaultFBO->attachDepthTexture(Window::getFrameSize(), GL_NEAREST, GL_CLAMP_TO_EDGE); // depth
    defaultFBO->initDrawBuffers();
    // Handle frame size changing
    Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
        const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
        glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
        Library::getFBO("default")->resize(frameSize);
    });
 
    Renderer::init("shaders/");
    Renderer::setDefaultFBO("default");

    std::shared_ptr<int> frameScale = std::make_shared<int>(16);
    Renderer::addPreProcessShader<DofInfoShader>("dofinfo.vert", "dofinfo.frag");
    Renderer::addPreProcessShader<DofDownShader>("dofdown.vert", "dofdown.frag", frameScale);
    Renderer::addPreProcessShader<DofBlurShader>("dofblur.vert", "dofblur.frag", frameScale);
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<AlphaTestShader>();
    Renderer::addPostProcessShader<PostShader>("post.frag");
    Renderer::addPostProcessShader<GammaCorrectShader>();

    Engine::addImGuiFunc("DOF", [&]() {
        if (ImGui::SliderInt("Scale", frameScale.get(), 1, 16)) {
            Library::getFBO("dofdown")->resize(Window::getFrameSize() / *frameScale);
            Library::getFBO("dofblur")->resize(Window::getFrameSize() / *frameScale);
        }
    });

    /* Run */
    Engine::run();
    return 0;
}