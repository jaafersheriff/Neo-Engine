#include <Engine.hpp>

#include "Renderer/Shader/PhongShader.hpp"
#include "Renderer/Shader/PostProcessShader.hpp"

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
    GameObject *gameObject;

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos);
        Engine::addComponent<LightComponent>(gameObject, col, att);
    }
};

struct Renderable {
    GameObject *gameObject;

    Renderable(Mesh *mesh, Texture* texture, float amb, glm::vec3 diffuse, glm::vec3 spec) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, glm::vec3(0.f), glm::vec3(1.f));
        Engine::addComponent<MeshComponent>(gameObject, *mesh);
        Material material;
        material.mAmbient = glm::vec3(amb);
        material.mDiffuse = diffuse;
        material.mSpecular = spec;
        Engine::addComponent<renderable::PhongRenderable>(gameObject, *texture, material);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Post Process";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    Renderable r(Library::loadMesh("mr_krab.obj"), Library::loadTexture("mr_krab.png"), 0.2f, glm::vec3(1.f, 0.f, 1.f), glm::vec3(1.f));

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addSceneShader<PhongShader>();
    Renderer::addPostProcessShader<PostProcessShader>("DepthShader", std::string("depth.frag"));
    Renderer::addPostProcessShader<PostProcessShader>("BlueShader", std::string("blue.frag"));
    Renderer::addPostProcessShader<PostProcessShader>("InvertShader", std::string("invert.frag"));

    /* Attach ImGui panes */

    /* Run */
    Engine::run();
    return 0;
}
