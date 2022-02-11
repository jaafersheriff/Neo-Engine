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
            Shader("GBufferShader", vert, frag) {

            // Create gbuffer 
            auto gbuffer = Library::createFBO("gbuffer");

            TextureFormat format{ GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT };
            gbuffer->attachColorTexture(WindowSurface::getFrameSize(), format); // color
            gbuffer->attachColorTexture(WindowSurface::getFrameSize(), format); // diffuse
            gbuffer->attachDepthTexture(WindowSurface::getFrameSize(), GL_NEAREST, GL_REPEAT); // depth
            gbuffer->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                NEO_UNUSED(m);
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).mFrameSize;
                Library::getFBO("gbuffer")->resize(frameSize);
            });
        }

        virtual void render() override {
            auto fbo = Library::getFBO("gbuffer");
            fbo->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            bind();

            if (auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>()) {
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("V", camera->get<CameraComponent>()->getView());
            }

            for (auto& renderableIt : Engine::getComponentTuples<GBufferComponent, MeshComponent, SpatialComponent>()) {
                auto renderable = renderableIt->get<GBufferComponent>();
                loadUniform("M", renderableIt->get<SpatialComponent>()->getModelMatrix());
                loadUniform("N", renderableIt->get<SpatialComponent>()->getNormalMatrix());

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