#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Engine.hpp"

#include "SunComponent.hpp"

using namespace neo;

class GodRaySunShader : public Shader {

    public:

        GodRaySunShader(const std::string &vert, const std::string &frag) :
            Shader("GodRay Shader", vert, frag) {

            // Create godray 
            auto godray = Library::getFBO("godray");
            godray->generate();

            // Format for color buffers
            TextureFormat format = { GL_R16, GL_RED, GL_NEAREST, GL_REPEAT };
            godray->attachColorTexture(Window::getFrameSize(), 1, format); 
            godray->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("godray")->resize(frameSize);
            });
        }

        virtual void render(const CameraComponent &camera) override {
            auto fbo = Library::getFBO("godray");
            fbo->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            glm::ivec2 frameSize = Window::getFrameSize();
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

            bind();
            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());
            glm::mat4 Vi = camera.getView();
            Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
            Vi = glm::transpose(Vi);
            loadUniform("Vi", Vi);

            auto& mesh = *Library::getMesh("plane");

            for (auto& renderable : Engine::getComponents<SunComponent>()) {

                /* Bind mesh */
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                loadUniform("M", renderable->getGameObject().getComponentByType<SpatialComponent>()->getModelMatrix());

                loadUniform("center", renderable->getGameObject().getComponentByType<SpatialComponent>()->getPosition());

                /* DRAW */
                mesh.draw();
            }

            unbind();
        }
};
