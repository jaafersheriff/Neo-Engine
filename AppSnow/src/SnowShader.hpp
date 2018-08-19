#pragma once

#include "NeoEngine.hpp"
#include "SnowComponent.hpp"

#include "Shader/Shader.hpp"
#include "Util/GlHelper.hpp"

namespace neo {

    class SnowShader : public Shader {

        public:

            SnowShader(RenderSystem &rSystem, const std::string &vert, const std::string &frag) :
                Shader("Snow Shader", rSystem.APP_SHADER_DIR, vert, frag)
            {}

            virtual void render(const RenderSystem &renderSystem, const CameraComponent &camera) override {
                bind();

                /* Load snow */
                auto snow = NeoEngine::getComponents<SnowComponent>()[0];
                loadVector(getUniform("snowAngle"), snow->snowAngle);
                loadVector(getUniform("snowColor"), snow->snowColor);
                loadFloat(getUniform("snowSize"), snow->snowSize);
                loadFloat(getUniform("height"), snow->height);
                loadVector(getUniform("rimColor"), snow->rimColor);
                loadFloat(getUniform("rimPower"), snow->rimPower);

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
