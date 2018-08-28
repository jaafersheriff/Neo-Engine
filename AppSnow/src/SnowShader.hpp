#pragma once

#include "NeoEngine.hpp"
#include "SnowComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLHelper/GlHelper.hpp"

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
                loadUniform("snowAngle", snow->snowAngle);
                loadUniform("snowColor", snow->snowColor);
                loadUniform("snowSize", snow->snowSize);
                loadUniform("height", snow->height);
                loadUniform("rimColor", snow->rimColor);
                loadUniform("rimPower", snow->rimPower);

                /* Load Camera */
                loadUniform("P", camera.getProj());
                loadUniform("V", camera.getView());
                loadUniform("camPos", camera.getGameObject().getSpatial()->getPosition());

                /* Load light */
                auto lights = NeoEngine::getComponents<LightComponent>();
                if (lights.size()) {
                    loadUniform("lightPos", lights.at(0)->getGameObject().getSpatial()->getPosition());
                    loadUniform("lightCol", lights.at(0)->getColor());
                    loadUniform("lightAtt", lights.at(0)->getAttenuation());
                }

                for (auto model : renderSystem.getRenderables<SnowShader, RenderableComponent>()) {
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
