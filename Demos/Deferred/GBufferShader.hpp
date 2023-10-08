#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "GBufferComponent.hpp"

#include "Messaging/Messenger.hpp"

using namespace neo;

namespace Deferred {
    class GBufferShader : public Shader {

    public:

        GBufferShader(const std::string& vert, const std::string& frag) :
            Shader("GBuffer Shader", vert, frag) {

            // Create gbuffer 
            auto gbuffer = Library::createFBO("gbuffer");

            TextureFormat format{ GL_RGB, GL_RGB, GL_NEAREST, GL_CLAMP_TO_EDGE };
            gbuffer->attachColorTexture({ 1, 1 }, format); // normal
            gbuffer->attachColorTexture({ 1, 1 }, format); // color
            gbuffer->attachDepthTexture({ 1, 1 }, GL_NEAREST, GL_CLAMP_TO_EDGE);  // depth
            gbuffer->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<FrameSizeMessage, &GBufferShader::_onFrameSizeChanged>(this);
        }

        virtual void render(const ECS& ecs) override {
            auto fbo = Library::getFBO("gbuffer");
            fbo->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            bind();

            if (auto cameraTuple = ecs.getSingleView<MainCameraComponent, CameraComponent, SpatialComponent>()) {
                auto&& [cameraEntity, _, camera, spatial] = *cameraTuple;
                loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
                loadUniform("V", spatial.getView());
            }

            ecs.getView<GBufferComponent, MeshComponent, SpatialComponent>().each([this](auto entity, auto gbufferComponent, auto mesh, auto spatial) {
                loadUniform("M", spatial.getModelMatrix());
                loadUniform("N", spatial.getNormalMatrix());

                /* Bind diffuse map or material */
                loadUniform("ambientColor", gbufferComponent.mMaterial.mAmbient);
                loadUniform("diffuseColor", gbufferComponent.mMaterial.mDiffuse);
                loadTexture("diffuseMap", *gbufferComponent.mDiffuseMap);

                /* DRAW */
                mesh.mMesh.draw();
                });

            unbind();
        }

        void GBufferShader::_onFrameSizeChanged(const FrameSizeMessage& msg) {
            Library::getFBO("gbuffer")->resize(msg.mSize);
        }
    };
}