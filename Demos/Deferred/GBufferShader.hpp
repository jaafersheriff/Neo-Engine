#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "GBufferComponent.hpp"

#include "ECS/Messaging/Messenger.hpp"

using namespace neo;

class GBufferShader : public Shader {

    public:

        GBufferShader(const std::string &vert, const std::string &frag) :
            Shader("GBuffer Shader", vert, frag) {

            // Create gbuffer 
            auto gbuffer = Library::createFBO("gbuffer");

            TextureFormat format{ GL_RGB, GL_RGB, GL_NEAREST, GL_CLAMP_TO_EDGE };
            gbuffer->attachColorTexture({ 1, 1 }, format); // normal
            gbuffer->attachColorTexture({ 1, 1 }, format); // color
            gbuffer->attachDepthTexture({ 1, 1 }, GL_NEAREST, GL_CLAMP_TO_EDGE);  // depth
            gbuffer->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<FrameSizeMessage>(nullptr, [&](const Message &msg, ECS& ecs) {
                NEO_UNUSED(ecs);
                glm::uvec2 frameSize = (static_cast<const FrameSizeMessage &>(msg)).mSize;
                Library::getFBO("gbuffer")->resize(frameSize);
            });
        }

        virtual void render(const ECS& ecs) override {
            auto fbo = Library::getFBO("gbuffer");
            fbo->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            bind();

            if (auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>()) {
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("V", camera->get<CameraComponent>()->getView());
            }

            for (auto& renderableIt : ecs.getComponentTuples<GBufferComponent, MeshComponent, SpatialComponent>()) {
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