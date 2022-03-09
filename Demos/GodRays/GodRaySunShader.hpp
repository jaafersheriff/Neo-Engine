#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "SunComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"

using namespace neo;

namespace GodRays {
    class GodRaySunShader : public Shader {

    public:

        GodRaySunShader(const std::string& vert, const std::string& frag) :
            Shader("GodRaySun Shader", vert, frag) {

            // Create godray 
            // 0 used for base 
            TextureFormat format = { GL_R16, GL_RED, GL_LINEAR, GL_CLAMP_TO_EDGE };
            auto godray = Library::createFBO("godray");
            godray->attachColorTexture({ 1, 1 }, format);
            godray->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<FrameSizeMessage>(nullptr, [&](const Message& msg, ECS& ecs) {
                NEO_UNUSED(ecs);
                glm::ivec2 frameSize = (static_cast<const FrameSizeMessage&>(msg)).mSize;
                Library::getFBO("godray")->resize(frameSize / 2);
                });

        }

        virtual void render(const ECS& ecs) override {
            auto fbo = Library::getFBO("godray");
            fbo->bind();
            auto viewport = ecs.getSingleComponent<ViewportDetailsComponent>();
            glViewport(0, 0, viewport->mSize.x, viewport->mSize.y);
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT);

            bind();

            /* Load PV */
            if (auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>()) {
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("V", camera->get<CameraComponent>()->getView());
                glm::mat4 Vi = camera->get<CameraComponent>()->getView();
                Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
                Vi = glm::transpose(Vi);
                loadUniform("Vi", Vi);
            }

            for (auto& renderable : ecs.getComponents<SunComponent>()) {

                loadUniform("M", renderable->getGameObject().getComponentByType<SpatialComponent>()->getModelMatrix());
                loadUniform("center", renderable->getGameObject().getComponentByType<SpatialComponent>()->getPosition());

                /* DRAW */
                Library::getMesh("quad").mMesh->draw();
            }

            unbind();
        }
    };
}
