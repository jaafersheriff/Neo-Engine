#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"
#include "Renderer/Renderer.hpp"
#include "Window/Window.hpp"

#include "Messaging/Messenger.hpp"

class GBufferShader : public neo::Shader {

    public:

        GBufferShader(const std::string &vert, const std::string &frag) :
            neo::Shader("GBufferShader", vert, frag) {

            // Create gbuffer 
            auto gbuffer = neo::Library::getFBO("gbuffer");
            gbuffer->generate();

            neo::TextureFormat format{ GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT };
            gbuffer->attachColorTexture(neo::Window::getFrameSize(), 4, format); // color
            gbuffer->attachColorTexture(neo::Window::getFrameSize(), 4, format); // diffuse
            gbuffer->attachDepthTexture(neo::Window::getFrameSize(), GL_NEAREST, GL_REPEAT); // depth
            gbuffer->initDrawBuffers();

            // Handle frame size changing
            neo::Messenger::addReceiver<neo::WindowFrameSizeMessage>(nullptr, [&](const neo::Message &msg) {
                const neo::WindowFrameSizeMessage & m(static_cast<const neo::WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const neo::WindowFrameSizeMessage &>(msg)).frameSize;
                neo::Library::getFBO("gbuffer")->resize(frameSize);
            });
        }

        virtual void render(const neo::CameraComponent &camera) override {
            auto fbo = neo::Library::getFBO("gbuffer");
            fbo->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            bind();
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            for (auto& renderable : neo::Engine::getComponents<neo::MeshComponent>()) {
                /* Bind mesh */
                const neo::Mesh& mesh(renderable->getMesh());
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                loadUniform("M", renderable->getGameObject().getSpatial()->getModelMatrix());

                /* Bind diffuse map or material */
                auto matComp = renderable->getGameObject().getComponentByType<neo::MaterialComponent>();
                if (matComp) {
                    loadUniform("ambient", matComp->mAmbient);
                }
                if (auto diffMap = renderable->getGameObject().getComponentByType<neo::DiffuseMapComponent>()) {
                    diffMap->mTexture->bind();
                    loadUniform("useDiffuseMap", true);
                    loadUniform("diffuseMap", diffMap->mTexture->mTextureID);
                }
                else {
                    loadUniform("useDiffuseMap", false);
                    if (matComp) {
                        loadUniform("diffuseMaterial", matComp->mDiffuse);
                    }
                }

                /* Bind normal map */
                if (auto normalMap = renderable->getGameObject().getComponentByType<neo::NormalMapComponent>()) {
                    normalMap->mTexture->bind();
                    loadUniform("useNormalMap", true);
                    loadUniform("normalMap", normalMap->mTexture->mTextureID);
                }
                else {
                    loadUniform("useNormalMap", false);
                    loadUniform("N", renderable->getGameObject().getSpatial()->getNormalMatrix());
                }

                /* DRAW */
                mesh.draw();
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            unbind();
    }
};