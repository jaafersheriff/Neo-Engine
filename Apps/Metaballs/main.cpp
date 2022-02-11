#include "Engine/Engine.hpp"

#include "DirtyBallsComponent.hpp"
#include "MetaballComponent.hpp"
#include "MetaballsMeshComponent.hpp"
#include "MetaballsSystem.hpp"
#include "MetaballsShader.hpp"

#include "Renderer/Shader/SkyboxShader.hpp"

using namespace neo;

/* Game object definitions */
struct Camera {
    CameraComponent *camera;
    Camera(float fov, float near, float far, float ls, float ms, glm::vec3 pos, glm::vec3 lookDir) {
        GameObject *gameObject = &Engine::createGameObject();
        auto& spatial = Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        spatial.setLookDir(lookDir);
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov, WindowSurface::getAspectRatio());
        Engine::addComponent<CameraControllerComponent>(gameObject, ls, ms);
    }
};

struct Metaball {
    Metaball(glm::vec3 position, float radius) {
        GameObject* gameObject = &Engine::createGameObject();
        Engine::addComponent<MetaballComponent>(gameObject);
        Engine::addComponent<MeshComponent>(gameObject, *Library::getMesh("sphere"));
        Engine::addComponent<SpatialComponent>(gameObject, position, glm::vec3(radius));
        Engine::addComponent<SelectableComponent>(gameObject);
        Engine::addComponent<BoundingBoxComponent>(gameObject, *Library::getMesh("sphere"));
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Metaballs";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, 0.4f, 7.f, glm::vec3(-2.f, 2.f, 50.f), glm::vec3(0.f, 0.f, -1.f));
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    /* Skybox */
    {
        GameObject* gameObject = &Engine::createGameObject();
        Engine::addComponent<renderable::SkyboxComponent>(gameObject, *Library::loadCubemap("arctic_skybox", {"arctic_ft.tga", "arctic_bk.tga", "arctic_up.tga", "arctic_dn.tga", "arctic_rt.tga", "arctic_lf.tga"}));
    }

    /* METBALL */
    {
        // Balls
        Metaball(glm::vec3(-1.4f, 0.f, 0.f), 1.f);
        Metaball(glm::vec3(1.4f, 0.f, 0.f), 2.5f);

        // Mesh
        auto& go = Engine::createGameObject();
        auto& mesh = Engine::addComponent<MetaballsMeshComponent>(&go);
        mesh.mMesh->mPrimitiveType = GL_TRIANGLES;
        mesh.mMesh->addVertexBuffer(VertexType::Position, 0, 3);
        mesh.mMesh->addVertexBuffer(VertexType::Normal, 1, 3);
        Engine::addComponent<SpatialComponent>(&go, glm::vec3(0.f, 0.f, 0.f));
    }

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<MetaballsSystem>();

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addSceneShader<MetaballsShader>("metaballs.vert", "metaballs.frag");
    Renderer::addSceneShader<SkyboxShader>();

    Engine::addImGuiFunc("Metaballs", []() {
        static float scale = 2.f;
        if (ImGui::Button("Add")) {
            Metaball(Util::genRandomVec3(-2.f, 2.f), Util::genRandom(2.f, 4.f));
            {
                GameObject* gameObject = &Engine::createGameObject();
                Engine::addComponent<DirtyBallsComponent>(gameObject);
            }
        }

        static int index = 0;
        auto metaballs = Engine::getComponents<MetaballComponent>();
        if (metaballs.size()) {
            ImGui::SliderInt("Index", &index, 0, static_cast<int>(metaballs.size()) - 1);
            glm::vec3 position = metaballs[index]->getGameObject().getComponentByType<SpatialComponent>()->getPosition();
            ImGui::Text("%0.2f, %0.2f, %0.2f", position.x, position.y, position.z);
            if (ImGui::Button("Remove")) {
                Engine::removeGameObject(metaballs[index]->getGameObject());
                {
                    GameObject* gameObject = &Engine::createGameObject();
                    Engine::addComponent<DirtyBallsComponent>(gameObject);
                }
                if (metaballs.size() - 1 == 1) {
                    index = 0;
                }
                else if (index == metaballs.size() - 2) {
                    index--;
                }
            }
        }
    });

    /* Run */
    Engine::run();
    return 0;
}