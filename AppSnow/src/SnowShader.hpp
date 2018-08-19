#pragma once

#include "NeoEngine.hpp"

#include "Shader/Shader.hpp"
#include "Util/GlHelper.hpp"

namespace neo {

    class SnowShader : public Shader {

        public:

            glm::vec3 snowAngle = glm::vec3(0.f, 1.f, 0.f);   // todo - attach to a spatial or messaging
            float snowSize = 0.f;
            glm::vec3 snowColor = glm::vec3(1.f);

            SnowShader(RenderSystem &rSystem, const std::string &vert, const std::string &frag) :
                Shader("Snow Shader", rSystem.APP_SHADER_DIR, vert, frag)
            {}

            virtual void render(const RenderSystem &renderSystem, const CameraComponent &camera) override {
                bind();

                /* Load snow */
                loadVector(getUniform("snowAngle"), snowAngle);
                loadVector(getUniform("snowColor"), snowColor);
                loadFloat(getUniform("snowSize"), snowSize);

                /* Load Camera */
                loadMatrix(getUniform("P"), camera.getProj());
                loadMatrix(getUniform("V"), camera.getView());
                loadVector(getUniform("camPos"), camera.getGameObject().getSpatial()->getPosition());

                /* Load light */
                auto lights = NeoEngine::getComponents<LightComponent>();
                if (lights.size()) {
                    loadVector(getUniform("lightPos"), lights.at(0)->getGameObject().getSpatial()->getPosition());
                    loadVector(getUniform("lightCol"), lights.at(0)->getColor());
                    loadVector(getUniform("lightAtt"), lights.at(0)->getAttenuation());
                }

                for (auto model : renderSystem.getRenderables<SnowShader, RenderableComponent>()) {
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
