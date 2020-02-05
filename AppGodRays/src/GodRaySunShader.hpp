#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Engine.hpp"

#include "SunComponent.hpp"

using namespace neo;

class GodRaySunShader : public Shader {

    public:

        GodRaySunShader(const std::string &vert, const std::string &frag) :
            Shader("GodRaySun Shader", vert, frag) {

            // Create godray 
            // 0 used for base 
            TextureFormat format = { GL_R16, GL_RED, GL_LINEAR, GL_CLAMP_TO_EDGE };
            auto godray = Library::getFBO("godray");
            godray->attachColorTexture(Window::getFrameSize() / 2, format); 
            godray->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::ivec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("godray")->resize(frameSize / 2);
            });

        }

        virtual void render() override {
            auto fbo = Library::getFBO("godray");
            fbo->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            glm::ivec2 frameSize = Window::getFrameSize() / 2;
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

            bind();

            /* Load PV */
            if (auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>()) {
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("V", camera->get<CameraComponent>()->getView());
                glm::mat4 Vi = camera->get<CameraComponent>()->getView();
                Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
                Vi = glm::transpose(Vi);
                loadUniform("Vi", Vi);
            }

            for (auto& renderable : Engine::getComponents<SunComponent>()) {

                loadUniform("M", renderable->getGameObject().getComponentByType<SpatialComponent>()->getModelMatrix());
                loadUniform("center", renderable->getGameObject().getComponentByType<SpatialComponent>()->getPosition());

                /* DRAW */
                Library::getMesh("plane")->draw();
            }

            unbind();
        }
};
