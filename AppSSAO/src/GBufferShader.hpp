#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"
#include "Window/Window.hpp"

#include "Messaging/Messenger.hpp"

using namespace neo;

class GBufferShader : public Shader {

    public:

        GBufferShader(const std::string &vert, const std::string &frag) :
            Shader("GBuffer Shader", vert, frag) {

            // Create gbuffer 
            auto gbuffer = Library::getFBO("gbuffer");
            gbuffer->generate();

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

            for (auto& model : Engine::getComponents<MeshComponent>()) {
                loadUniform("M", model->getGameObject().getComponentByType<SpatialComponent>()->getModelMatrix());

                /* Bind diffuse map or material */
                auto matComp = model->getGameObject().getComponentByType<MaterialComponent>();
                if (matComp) {
                    loadUniform("ambient", matComp->mAmbient);
                }
                if (auto diffMap = model->getGameObject().getComponentByType<DiffuseMapComponent>()) {
                    loadTexture("diffuseMap", diffMap->mTexture);
                    loadUniform("useDiffuseMap", true);
                }
                else {
                    loadUniform("useDiffuseMap", false);
                    if (matComp) {
                        loadUniform("diffuseMaterial", matComp->mDiffuse);
                    }
                }

                /* Bind normal map */
                if (auto normalMap = model->getGameObject().getComponentByType<NormalMapComponent>()) {
                    loadTexture("normalMap", normalMap->mTexture);
                    loadUniform("useNormalMap", true);
                }
                else {
                    loadUniform("useNormalMap", false);
                    loadUniform("N", model->getGameObject().getComponentByType<SpatialComponent>()->getNormalMatrix());
                }

                /* DRAW */
                model->getMesh().draw();
            }

            unbind();
    }
};