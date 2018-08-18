#pragma once

#include "Shader/Shader.hpp"
#include "System/RenderSystem/Framebuffer.hpp"
#include "System/RenderSystem/RenderSystem.hpp"

using namespace neo;

class ShadowCasterShader : public Shader {

    public:

        ShadowCasterShader(RenderSystem &rSystem, const std::string &vert, const std::string &frag) :
            Shader("Shadow Caster", rSystem, vert, frag) {

            /* Init shadow map */
            Texture *depthTexture = Loader::getTexture("depthTexture");
            depthTexture->width = 1024;
            depthTexture->height = 1024;
            depthTexture->components = 1;
            depthTexture->upload(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_NEAREST, GL_REPEAT);

            Framebuffer *depthFBO = rSystem.createFBO("depthMap");
            depthFBO->generate();
            depthFBO->attachDepthTexture(*depthTexture);
            depthFBO->disableDraw();
            depthFBO->disableRead();
        }

        virtual void render(const RenderSystem &rSystem, const CameraComponent &camera) override {
            rSystem.framebuffers.find("depthMap")->second->bind();
            auto depthTexture = Loader::getTexture("depthTexture");
            CHECK_GL(glClear(GL_DEPTH_BUFFER_BIT));
            CHECK_GL(glViewport(0, 0, depthTexture->width, depthTexture->height));
            bind();

            auto cameras = NeoEngine::getComponents<LightComponent>()[0]->getGameObject().getComponentsByType<CameraComponent>();
            if (cameras.size()) {
                loadMatrix(getUniform("P"), cameras[0]->getProj());
                loadMatrix(getUniform("V"), cameras[0]->getView());
            }

            for (auto model : rSystem.getRenderables<ShadowCasterShader, RenderableComponent>()) {
                loadMatrix(getUniform("M"), model->getGameObject().getSpatial()->getModelMatrix());

                /* Bind mesh */
                const Mesh & mesh(model->getMesh());
                CHECK_GL(glBindVertexArray(mesh.vaoId));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                /* DRAW */
                CHECK_GL(glDrawElements(GL_TRIANGLES, (int)mesh.eleBufSize, GL_UNSIGNED_INT, nullptr));
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            unbind();
        }
};












