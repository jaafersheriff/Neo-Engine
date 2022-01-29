#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Window/Window.hpp"

#include "GBufferComponent.hpp"

#include "Messaging/Messenger.hpp"

using namespace neo;

class GBufferShader : public Shader {

    public:

        GBufferShader(const std::string &vert, const std::string &frag) :
            Shader("GBuffer Shader", vert, frag) {

            // Create gbuffer 
            auto gbuffer = Library::createFBO("gbuffer");

            TextureFormat format{ GL_RGB, GL_RGB, GL_NEAREST, GL_CLAMP_TO_EDGE };
            gbuffer->attachColorTexture(Window::getFrameSize(), format); // normal
            gbuffer->attachColorTexture(Window::getFrameSize(), format); // color
            gbuffer->attachDepthTexture(Window::getFrameSize(), GL_NEAREST, GL_CLAMP_TO_EDGE);  // depth
            gbuffer->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("gbuffer")->resize(frameSize);
            });
        }

        virtual void render() override {
            auto fbo = Library::getFBO("gbuffer");
            fbo->bind();
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            bind();

            if (auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>()) {
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("V", camera->get<CameraComponent>()->getView());
            }

            for (auto& renderableIt : Engine::getComponentTuples<GBufferComponent, MeshComponent, SpatialComponent>()) {
                auto renderable = renderableIt->get<GBufferComponent>();
                auto spatial = renderableIt->get<SpatialComponent>();

                loadUniform("M", spatial->getModelMatrix());
                loadUniform("N", spatial->getNormalMatrix());

                /* Bind diffuse map or material */
                loadUniform("ambientColor", renderable->mMaterial.mAmbient);
                loadUniform("diffuseColor", renderable->mMaterial.mDiffuse);
                loadTexture("diffuseMap", renderable->mDiffuseMap);

                /* DRAW */
                renderableIt->get<MeshComponent>()->mMesh.draw();
            }

            unbind();
    }
};