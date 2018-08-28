#pragma once

#include "NeoEngine.hpp"

#include "Shader/Shader.hpp"
#include "GLHelper/GlHelper.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    class ShadowReceiverShader : public Shader {

        public:

            ShadowReceiverShader(RenderSystem &rSystem, const std::string &vert, const std::string &frag, float b = 0.f) :
                Shader("ShadowReceiver Shader", rSystem.APP_SHADER_DIR, vert, frag),
                bias(b)
            {}

            float bias;

            virtual void render(const RenderSystem &renderSystem, const CameraComponent &camera) override {
                bind();

                /* Load Camera */
                loadUniform("P", camera.getProj());
                loadUniform("V", camera.getView());
                loadUniform("camPos", camera.getGameObject().getSpatial()->getPosition());

                /* Load light */
                auto lights = NeoEngine::getComponents<LightComponent>();
                if (lights.size()) {
                    auto light = lights[0];
                    auto lightCam = light->getGameObject().getComponentByType<CameraComponent>();
                    loadUniform("lightPos", light->getGameObject().getSpatial()->getPosition());
                    loadUniform("lightCol", light->getColor());
                    loadUniform("lightAtt", light->getAttenuation());
                    loadUniform("lightP", lightCam->getProj());
                    loadUniform("lightV", lightCam->getView());
                }

                /* Bias */
                loadUniform("bias", bias);

                /* Bind shadow map */
                const Texture & texture(*renderSystem.framebuffers.find("depthMap")->second.get()->textures[0]);
                CHECK_GL(glActiveTexture(GL_TEXTURE0 + texture.textureId));
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, texture.textureId));
                loadUniform("shadowMap", texture.textureId);

                for (auto model : renderSystem.getRenderables<ShadowReceiverShader, RenderableComponent>()) {
                    loadUniform("M", model->getGameObject().getSpatial()->getModelMatrix());
                    loadUniform("N", model->getGameObject().getSpatial()->getNormalMatrix());

                    /* Bind mesh */
                    const Mesh & mesh(model->getMesh());
                    CHECK_GL(glBindVertexArray(mesh.vaoId));
                    CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                    /* Bind material */
                    auto materialComp = model->getGameObject().getComponentByType<MaterialComponent>();
                    if (materialComp) {
                        const Material &material = materialComp->getMaterial();
                        loadUniform("ambient", material.ambient);
                        loadUniform("diffuseColor", material.diffuse);
                        loadUniform("specularColor", material.specular);
                        loadUniform("shine", material.shine);
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
}
