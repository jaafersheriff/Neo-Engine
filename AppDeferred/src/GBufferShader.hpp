#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/GLHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"
#include "Window/Window.hpp"

#include "Messaging/Messenger.hpp"

using namespace neo;

class GBufferShader : public Shader {

    public:

        GBufferShader(const std::string &vert, const std::string &frag) :
            Shader("GBufferShader", vert, frag) {

            // Create gbuffer 
            auto gbuffer = Loader::getFBO("gbuffer");
            gbuffer->generate();
            gbuffer->attachColorTexture(Window::getFrameSize(), 4, GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT); // color
            gbuffer->attachColorTexture(Window::getFrameSize(), 4, GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT); // diffuse
            gbuffer->attachDepthTexture(Window::getFrameSize(), GL_NEAREST, GL_REPEAT); // depth
            gbuffer->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::ivec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                auto gbuffer = Loader::getFBO("gbuffer");
                gbuffer->mTextures[0]->mWidth = gbuffer->mTextures[1]->mWidth = gbuffer->mTextures[2]->mWidth = frameSize.x;
                gbuffer->mTextures[0]->mHeight = gbuffer->mTextures[1]->mHeight = gbuffer->mTextures[2]->mHeight = frameSize.y;
                gbuffer->mTextures[0]->bind();
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frameSize.x, frameSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
                gbuffer->mTextures[1]->bind();
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frameSize.x, frameSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
                gbuffer->mTextures[2]->bind();
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, frameSize.x, frameSize.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr));
            });
        }

        virtual void render(const CameraComponent &camera) override {
            auto fbo = Loader::getFBO("gbuffer");
            fbo->bind();
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            bind();
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            for (auto & model : MasterRenderer::getRenderables<GBufferShader, RenderableComponent>()) {
                loadUniform("M", model->getGameObject().getSpatial()->getModelMatrix());

                /* Bind mesh */
                const Mesh & mesh(model->getMesh());
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                /* Bind diffuse map or material */
                auto matComp = model->getGameObject().getComponentByType<MaterialComponent>();
                if (matComp) {
                    loadUniform("ambient", matComp->getMaterial().mAmbient);
                }
                auto diffMap = model->getGameObject().getComponentByType<DiffuseMapComponent>();
                if (diffMap) {
                    diffMap->getTexture().bind();
                    loadUniform("useDiffuseMap", true);
                    loadUniform("diffuseMap", diffMap->getTexture().mTextureID);
                }
                else {
                    loadUniform("useDiffuseMap", false);
                    if (matComp) {
                        loadUniform("diffuseMaterial", matComp->getMaterial().mDiffuse);
                    }
                }

                /* Bind normal map */
                auto normalMap = model->getGameObject().getComponentByType<NormalMapComponent>();
                if (normalMap) {
                    normalMap->getTexture().bind();
                    loadUniform("useNormalMap", true);
                    loadUniform("normalMap", normalMap->getTexture().mTextureID);
                }
                else {
                    loadUniform("useNormalMap", false);
                    loadUniform("N", model->getGameObject().getSpatial()->getNormalMatrix());
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