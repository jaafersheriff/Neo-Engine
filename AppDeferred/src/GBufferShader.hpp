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
            Shader("GBuffer Shader", vert, frag) {

            // Create gbuffer 
            auto gbuffer = Loader::getFBO("gbuffer");
            gbuffer->generate();
            gbuffer->attachColorTexture(Window::getFrameSize(), 4, GL_RGB, GL_RGB, GL_NEAREST, GL_CLAMP_TO_EDGE); // normal
            gbuffer->attachColorTexture(Window::getFrameSize(), 4, GL_RGB, GL_RGB, GL_NEAREST, GL_CLAMP_TO_EDGE); // color
            gbuffer->attachDepthTexture(Window::getFrameSize(), GL_NEAREST, GL_CLAMP_TO_EDGE);                    // depth
            gbuffer->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::ivec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                auto gbuffer = Loader::getFBO("gbuffer");
                gbuffer->textures[0]->width = gbuffer->textures[1]->width = gbuffer->textures[2]->width = frameSize.x;
                gbuffer->textures[0]->height = gbuffer->textures[1]->height = gbuffer->textures[2]->height = frameSize.y;
                gbuffer->textures[0]->bind();
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameSize.x, frameSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));
                gbuffer->textures[1]->bind();
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameSize.x, frameSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));
                gbuffer->textures[2]->bind();
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
                CHECK_GL(glBindVertexArray(mesh.vaoId));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                /* Bind diffuse map or material */
                auto matComp = model->getGameObject().getComponentByType<MaterialComponent>();
                if (matComp) {
                    loadUniform("ambient", matComp->getMaterial().ambient);
                }
                auto diffMap = model->getGameObject().getComponentByType<DiffuseMapComponent>();
                if (diffMap) {
                    diffMap->getTexture().bind();
                    loadUniform("useDiffuseMap", true);
                    loadUniform("diffuseMap", diffMap->getTexture().textureId);
                }
                else {
                    loadUniform("useDiffuseMap", false);
                    if (matComp) {
                        loadUniform("diffuseMaterial", matComp->getMaterial().diffuse);
                    }
                }

                /* Bind normal map */
                auto normalMap = model->getGameObject().getComponentByType<NormalMapComponent>();
                if (normalMap) {
                    normalMap->getTexture().bind();
                    loadUniform("useNormalMap", true);
                    loadUniform("normalMap", normalMap->getTexture().textureId);
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