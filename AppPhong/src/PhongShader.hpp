#pragma once

#include "NeoEngine.hpp"

#include "Shader/Shader.hpp"
#include "Util/GlHelper.hpp"

using namespace neo;

class PhongShader : public Shader {

    public: 
    
        PhongShader(const std::string &res, const std::string &vert, const std::string &frag) :
            Shader("Phong Shader", res, vert, frag) 
        {}

        virtual void render(const RenderSystem &renderSystem, const CameraComponent &camera) override {
            bind();

            /* Load PV */
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

            for (auto model : renderSystem.getRenderables<PhongShader, RenderableComponent>()) {
                loadMatrix(getUniform("M"), model->getGameObject().getSpatial()->getModelMatrix());
                loadMatrix(getUniform("N"), model->getGameObject().getSpatial()->getNormalMatrix());

                /* Bind mesh */
                const Mesh & mesh(model->getMesh());
                CHECK_GL(glBindVertexArray(mesh.vaoId));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                /* Bind texture */
                auto texComp = model->getGameObject().getComponentByType<TextureComponent>();
                if (texComp) {
                    const Texture & texture(texComp->getTexture());
                    CHECK_GL(glActiveTexture(GL_TEXTURE0 + texture.textureId));
                    CHECK_GL(glBindTexture(GL_TEXTURE_2D, texture.textureId));
                    loadInt(getUniform("diffuseMap"), texture.textureId);
                }

                /* Bind material */
                auto matComp = model->getGameObject().getComponentByType<MaterialComponent>();
                if (matComp) {
                    const Material & material(matComp->getMaterial());
                    loadFloat(getUniform("ambient"), material.ambient);
                    loadVector(getUniform("specularColor"), material.specular);
                    loadFloat(getUniform("shine"), material.shine);
                }

                /* DRAW */
                CHECK_GL(glDrawElements(GL_TRIANGLES, (int)mesh.eleBufSize, GL_UNSIGNED_INT, nullptr));
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            CHECK_GL(glActiveTexture(GL_TEXTURE0));
            CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));
            unbind();
        }
};
