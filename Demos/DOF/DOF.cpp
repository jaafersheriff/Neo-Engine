#include "DOF/DOF.hpp"
#include "Engine/Engine.hpp"

#include "Renderer/Shader/PhongShadowShader.hpp"
#include "Renderer/Shader/ShadowCasterShader.hpp"
#include "Renderer/Shader/SkyboxShader.hpp"
#include "Renderer/Shader/GammaCorrectShader.hpp"

#include "Loader/Loader.hpp"

#include "DofBlurShader.hpp"
#include "DofInfoShader.hpp"
#include "DofDownShader.hpp"
#include "PostShader.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/RenderableComponent/WireframeRenderable.hpp"
#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
namespace {
    struct Camera {
        CameraComponent* camera;
        Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
            GameObject* gameObject = &ecs.createGameObject("Camera");
            ecs.addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
            camera = &ecs.addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
            ecs.addComponent<CameraControllerComponent>(gameObject, ls, ms);
        }
    };

    struct Light {
        GameObject* gameObject;
        Light(ECS& ecs, glm::vec3 pos, glm::vec3 col, glm::vec3 att, glm::vec3 lookDir) {
            gameObject = &ecs.createGameObject("Light");
            auto& spat = ecs.addComponent<SpatialComponent>(gameObject, pos);
            spat.setLookDir(lookDir);
            ecs.addComponent<LightComponent>(gameObject, col, att);
            ecs.addComponentAs<OrthoCameraComponent, CameraComponent>(gameObject, -1.f, 1000.f, -100.f, 100.f, -100.f, 100.f);
            ecs.addComponent<ShadowCameraComponent>(gameObject);
            ecs.addComponent<SelectableComponent>(gameObject);
            ecs.addComponent<renderable::WireframeRenderable>(gameObject);

            Engine::addImGuiFunc("Light", [](ECS& ecs_) {
                auto light = ecs_.getSingleComponent<LightComponent>();
                light->imGuiEditor();
                if (auto spatial = light->getGameObject().getComponentByType<SpatialComponent>()) {
                    spatial->imGuiEditor();
                }
                });
        }
    };

    struct Renderable {
        GameObject* gameObject;

        Renderable(ECS& ecs, Mesh* mesh, glm::vec3 position = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f), glm::vec3 rotation = glm::vec3(0.f)) {
            gameObject = &ecs.createGameObject("sponza");
            ecs.addComponent<MeshComponent>(gameObject, *mesh);
            ecs.addComponent<SpatialComponent>(gameObject, position, scale, rotation);
        }
    };
}

IDemo::Config DOF::getConfig() const {
    IDemo::Config config;
    config.name = "DOF";
    config.attachEditor = true;
    return config;
}

void DOF::init(ECS& ecs) {

    /* Game objects */
    Camera camera(ecs, 45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    ecs.addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light light(ecs, glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f), glm::normalize(glm::vec3(0.43f, -0.464f, -0.776f)));

    /* Scene objects */
    // for (int i = 0; i < 50; i++) {
    //     auto mesh = i % 2 ? Library::getMesh("cube") : Library::getMesh("sphere");
    //     Renderable renderable(ecs, mesh, glm::vec3(util::genRandom(-30.f, 30.f), 1.f, util::genRandom(-30.f, 30.f)), glm::vec3(util::genRandom(1.5f, 4.5f)), util::genRandomVec3(-util::PI, util::PI));
    //     Material material;
    //     material.mAmbient = glm::vec3(0.2f);
    //     material.mDiffuse = util::genRandomVec3(0.2f, 1.f);
    //     ecs.addComponent<renderable::PhongShadowRenderable>(renderable.gameObject, *Library::getTexture("black"), material);
    //     ecs.addComponent<renderable::ShadowCasterRenderable>(renderable.gameObject, *Library::getTexture("black"));
    //     ecs.addComponent<SelectableComponent>(renderable.gameObject);
    //     ecs.addComponent<BoundingBoxComponent>(renderable.gameObject, *mesh);
    // }

    {
        auto mesh = Library::loadMesh("sponza.obj");
        Renderable renderable(ecs, mesh, glm::vec3(0.f), glm::vec3(0.2f));
        Material material;
        material.mAmbient = glm::vec3(0.2f);
        material.mDiffuse = util::genRandomVec3(0.2f, 1.f);
        ecs.addComponent<renderable::PhongShadowRenderable>(renderable.gameObject, *Library::getTexture("black"), material);
        ecs.addComponent<renderable::ShadowCasterRenderable>(renderable.gameObject, *Library::getTexture("black"));
        ecs.addComponent<BoundingBoxComponent>(renderable.gameObject, *mesh);

    }

    /* Ground plane */
    {
        // Renderable plane(ecs, Library::getMesh("quad"), glm::vec3(0.f), glm::vec3(100.f), glm::vec3(-util::PI / 2.f, 0.f, 0.f));
        // Material material;
        // material.mAmbient = glm::vec3(0.2f);
        // material.mDiffuse = glm::vec3(0.2f);
        // ecs.addComponent<renderable::PhongShadowRenderable>(plane.gameObject, *Library::getTexture("black"), material);
    }

    /* Skybox */
    {
        // GameObject* gameObject = &ecs.createGameObject("Skybox");
        // ecs.addComponent<renderable::SkyboxComponent>(gameObject, *Library::loadCubemap("arctic_skybox", {"arctic_ft.tga", "arctic_bk.tga", "arctic_up.tga", "arctic_dn.tga", "arctic_rt.tga", "arctic_lf.tga"}));
    }

    /* Systems - order matters! */
    ecs.addSystem<CameraControllerSystem>();

    /* Init renderer */
    auto defaultFBO = Library::createFBO("default");
    defaultFBO->attachColorTexture({ 1, 1 }, { GL_RGBA, GL_RGBA, GL_NEAREST, GL_CLAMP_TO_EDGE });
    defaultFBO->attachDepthTexture({ 1, 1 }, GL_NEAREST, GL_CLAMP_TO_EDGE); // depth
    defaultFBO->initDrawBuffers();
    // Handle frame size changing
    Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg, ECS& ecs) {
        NEO_UNUSED(ecs);
        glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).mFrameSize;
        Library::getFBO("default")->resize(frameSize);
    });
    Renderer::setDefaultFBO("default");

    std::shared_ptr<int> frameScale = std::make_shared<int>(16);
    Renderer::addPreProcessShader<DofInfoShader>("dof/dofinfo.vert", "dof/dofinfo.frag");
    Renderer::addPreProcessShader<DofDownShader>("dof/dofdown.vert", "dof/dofdown.frag", frameScale);
    Renderer::addPreProcessShader<DofBlurShader>("dof/dofblur.vert", "dof/dofblur.frag", frameScale);
    Renderer::addPreProcessShader<ShadowCasterShader>(4096);
    auto& phongshadow = Renderer::addSceneShader<PhongShadowShader>();
    phongshadow.bias = 0.002f;
    Renderer::addSceneShader<SkyboxShader>();
    Renderer::addPostProcessShader<PostShader>("dof/post.frag");
    Renderer::addPostProcessShader<GammaCorrectShader>();

    Engine::addImGuiFunc("DOF", [frameScale](ECS& ecs_) {
        if (auto windowDetails = ecs_.getSingleComponent<WindowDetailsComponent>()) {
            if (ImGui::SliderInt("Scale", frameScale.get(), 1, 16)) {
                Library::getFBO("dofdown")->resize(windowDetails->mDetails.getSize() / *frameScale);
                Library::getFBO("dofblur")->resize(windowDetails->mDetails.getSize() / *frameScale);
            }
        }
    });
;
}