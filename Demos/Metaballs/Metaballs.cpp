#include "Metaballs.hpp"
#include "Engine/Engine.hpp"

#include "DirtyBallsComponent.hpp"
#include "MetaballComponent.hpp"
#include "MetaballsMeshComponent.hpp"
#include "MetaballsSystem.hpp"
#include "MetaballsShader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"

#include "Renderer/Shader/SkyboxShader.hpp"

using namespace neo;

/* Game object definitions */
namespace Metaballs {
    struct Camera {
        CameraComponent* camera;
        Camera(ECS& ecs, float fov, float near, float far, float ls, float ms, glm::vec3 pos, glm::vec3 lookDir) {
            GameObject* gameObject = &ecs.createGameObject();
            auto& spatial = ecs.addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
            spatial.setLookDir(lookDir);
            camera = &ecs.addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
            ecs.addComponent<CameraControllerComponent>(gameObject, ls, ms);
        }
    };

    struct Metaball {
        Metaball(ECS& ecs, glm::vec3 position, float radius) {
            GameObject* gameObject = &ecs.createGameObject();
            ecs.addComponent<MetaballComponent>(gameObject);
            ecs.addComponent<MeshComponent>(gameObject, *Library::getMesh("sphere").mMesh);
            ecs.addComponent<SpatialComponent>(gameObject, position, glm::vec3(radius));
            ecs.addComponent<SelectableComponent>(gameObject);
            ecs.addComponent<BoundingBoxComponent>(gameObject, Library::getMesh("sphere"));
        }
    };

    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "Metaballs";
        return config;
    }

    void Demo::init(ECS& ecs) {

        /* Game objects */
        Camera camera(ecs, 45.f, 1.f, 100.f, 0.4f, 7.f, glm::vec3(-2.f, 2.f, 50.f), glm::vec3(0.f, 0.f, -1.f));
        ecs.addComponent<MainCameraComponent>(&camera.camera->getGameObject());

        /* Skybox */
        {
            GameObject* gameObject = &ecs.createGameObject();
            ecs.addComponent<renderable::SkyboxComponent>(gameObject, *Library::loadCubemap("arctic_skybox", { "arctic_ft.tga", "arctic_bk.tga", "arctic_up.tga", "arctic_dn.tga", "arctic_rt.tga", "arctic_lf.tga" }));
        }

        /* METBALL */
        {
            // Balls
            Metaball(ecs, glm::vec3(-1.4f, 0.f, 0.f), 1.f);
            Metaball(ecs, glm::vec3(1.4f, 0.f, 0.f), 2.5f);

            // Mesh
            auto& go = ecs.createGameObject();
            auto& mesh = ecs.addComponent<MetaballsMeshComponent>(&go);
            mesh.mMesh->mPrimitiveType = GL_TRIANGLES;
            mesh.mMesh->addVertexBuffer(VertexType::Position, 0, 3);
            mesh.mMesh->addVertexBuffer(VertexType::Normal, 1, 3);
            ecs.addComponent<SpatialComponent>(&go, glm::vec3(0.f, 0.f, 0.f));
        }

        /* Systems - order matters! */
        ecs.addSystem<CameraControllerSystem>();
        ecs.addSystem<MetaballsSystem>();

        /* Init renderer */
        Renderer::addSceneShader<MetaballsShader>("metaballs/metaballs.vert", "metaballs/metaballs.frag");
        Renderer::addSceneShader<SkyboxShader>();

        Engine::addImGuiFunc("Metaballs", [](ECS& ecs_) {
            static float scale = 2.f;
            if (ImGui::Button("Add")) {
                Metaball(ecs_, util::genRandomVec3(-2.f, 2.f), util::genRandom(2.f, 4.f));
                {
                    GameObject* gameObject = &ecs_.createGameObject();
                    ecs_.addComponent<DirtyBallsComponent>(gameObject);
                }
            }

            static int index = 0;
            auto metaballs = ecs_.getComponents<MetaballComponent>();
            if (metaballs.size()) {
                ImGui::SliderInt("Index", &index, 0, static_cast<int>(metaballs.size()) - 1);
                glm::vec3 position = metaballs[index]->getGameObject().getComponentByType<SpatialComponent>()->getPosition();
                ImGui::Text("%0.2f, %0.2f, %0.2f", position.x, position.y, position.z);
                if (ImGui::Button("Remove")) {
                    ecs_.removeGameObject(metaballs[index]->getGameObject());
                    {
                        GameObject* gameObject = &ecs_.createGameObject();
                        ecs_.addComponent<DirtyBallsComponent>(gameObject);
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
    }
}