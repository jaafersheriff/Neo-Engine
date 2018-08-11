#pragma once

#include "Shader/Shader.hpp"

#include "Util/GLHelper.hpp"

#include "NeoEngine.hpp"

#include "RefractionRenderable.hpp"
#include "SkyboxComponent.hpp"

using namespace neo;

class RefractionShader : public Shader {

    public:

        RefractionShader(const std::string &res, const std::string &vert, const std::string &frag) :
            Shader("Refraction Shader", res, vert, frag)
        {}

        virtual void render(float dt, const RenderSystem &renderSystem) override {
            bind();

            /* Load PV */
            const auto cameras = NeoEngine::getComponents<CameraComponent>();
            if (cameras.size()) {
                loadMatrix(getUniform("P"), cameras.at(0)->getProj());
                loadMatrix(getUniform("V"), cameras.at(0)->getView());
                loadVector(getUniform("camPos"), cameras.at(0)->getGameObject().getSpatial()->getPosition());
            }

            /* Load environment map */
            loadInt(getUniform("cubeMap"), NeoEngine::getComponents<SkyboxComponent>()[0]->getTexture().textureId);

            for (auto model : renderSystem.getRenderables<RefractionShader, RefractionRenderable>()) {
                loadMatrix(getUniform("M"), model->getGameObject().getSpatial()->getModelMatrix());
                loadMatrix(getUniform("N"), model->getGameObject().getSpatial()->getNormalMatrix());
                loadFloat(getUniform("ratio"), model->ratio);

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
            CHECK_GL(glActiveTexture(GL_TEXTURE0));
            CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));

            unbind();
        }
};