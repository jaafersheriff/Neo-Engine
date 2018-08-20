#pragma once

#include "Shader/Shader.hpp"

#include "Util/GLHelper.hpp"

#include "NeoEngine.hpp"

#include "RefractionRenderable.hpp"
#include "SkyboxComponent.hpp"

using namespace neo;

class RefractionShader : public Shader {

    public:

        RefractionShader(RenderSystem &r, const std::string &vert, const std::string &frag) :
            Shader("Refraction Shader", r.APP_SHADER_DIR, vert, frag)
        {}

        virtual void render(const RenderSystem &renderSystem, const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadMatrix(getUniform("P"), camera.getProj());
            loadMatrix(getUniform("V"), camera.getView());
            loadVector(getUniform("camPos"), camera.getGameObject().getSpatial()->getPosition());

            /* Load environment map */
            loadInt(getUniform("cubeMap"), NeoEngine::getComponents<SkyboxComponent>()[0]->getGameObject().getComponentByType<TextureComponent>()->getTexture().textureId);

            for (auto model : renderSystem.getRenderables<RefractionShader, RefractionRenderable>()) {
                loadMatrix(getUniform("M"), model->getGameObject().getSpatial()->getModelMatrix());
                loadMatrix(getUniform("N"), model->getGameObject().getSpatial()->getNormalMatrix());
                loadFloat(getUniform("ratio"), model->ratio);

                /* Bind mesh */
                const Mesh & mesh(model->getMesh());
                CHECK_GL(glBindVertexArray(mesh.vaoId));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                /* DRAW */
                mesh.draw();
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            CHECK_GL(glActiveTexture(GL_TEXTURE0));
            CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));

            unbind();
        }
};