#pragma once

#include "NeoEngine.hpp"

#include "Shader/Shader.hpp"
#include "Util/GlHelper.hpp"

namespace neo {

    class ShadowReceiverShader : public Shader {

        public:

            ShadowReceiverShader(RenderSystem &rSystem, const std::string &vert, const std::string &frag) :
                Shader("ShadowReceiver Shader", rSystem, vert, frag)
            {}

            virtual void render(const RenderSystem &renderSystem, const CameraComponent &camera) override {
                bind();

                /* Load Camera */
                loadMatrix(getUniform("P"), camera.getProj());
                loadMatrix(getUniform("V"), camera.getView());
                loadVector(getUniform("camPos"), camera.getGameObject().getSpatial()->getPosition());

                /* Load light */
                auto lights = NeoEngine::getComponents<LightComponent>();
                if (lights.size()) {
                    auto light = lights[0];
                    loadVector(getUniform("lightPos"), light->getGameObject().getSpatial()->getPosition());
                    loadVector(getUniform("lightCol"), light->getColor());
                    loadVector(getUniform("lightAtt"), light->getAttenuation());
                    auto lightCam = light->getGameObject().getComponentByType<CameraComponent>();
                    loadMatrix(getUniform("lightP"), lightCam->getProj());
                    loadMatrix(getUniform("lightV"), lightCam->getView());
                }

                /* Bind shadow map */
                const Texture & texture(*Loader::getTexture("depthTexture"));
                CHECK_GL(glActiveTexture(GL_TEXTURE0 + texture.textureId));
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, texture.textureId));
                loadInt(getUniform("shadowMap"), texture.textureId);

                for (auto model : renderSystem.getRenderables<ShadowReceiverShader, RenderableComponent>()) {
                    loadMatrix(getUniform("M"), model->getGameObject().getSpatial()->getModelMatrix());
                    loadMatrix(getUniform("N"), model->getGameObject().getSpatial()->getNormalMatrix());

                    /* Bind mesh */
                    const Mesh & mesh(model->getMesh());
                    CHECK_GL(glBindVertexArray(mesh.vaoId));
                    CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                    /* Bind material */
                    auto materialComp = model->getGameObject().getComponentByType<MaterialComponent>();
                    if (materialComp) {
                        const Material &material = materialComp->getMaterial();
                        loadFloat(getUniform("ambient"), material.ambient);
                        loadVector(getUniform("diffuseColor"), material.diffuse);
                        loadVector(getUniform("specularColor"), material.specular);
                        loadFloat(getUniform("shine"), material.shine);
                    }

                    /* DRAW */
                    CHECK_GL(glDrawElements(GL_TRIANGLES, (int)mesh.eleBufSize, GL_UNSIGNED_INT, nullptr));
                }

                CHECK_GL(glBindVertexArray(0));
                CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
                unbind();
            }
    };
}
