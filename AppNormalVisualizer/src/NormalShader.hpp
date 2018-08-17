#pragma once

#include "NeoEngine.hpp"

#include "Shader/Shader.hpp"
#include "Util/GlHelper.hpp"

using namespace neo;

class NormalShader : public Shader {

    public: 
    
        NormalShader(const std::string &res, const std::string &vert, const std::string &frag, const std::string &geom) :
            Shader("Normal Shader", res, vert, frag, geom) 
        {}

        virtual void render(const RenderSystem &renderSystem, const CameraComponent &camera) override {
            bind();

            loadFloat(getUniform("magnitude"), magnitude);

            /* Load PV */
            loadMatrix(getUniform("P"), camera.getProj());
            loadMatrix(getUniform("V"), camera.getView());

            for (auto model : renderSystem.getRenderables<NormalShader, RenderableComponent>()) {
                glm::mat4 M = model->getGameObject().getSpatial()->getModelMatrix();
                loadMatrix(getUniform("M"), M);
                glm::mat4 N = glm::transpose(glm::inverse(camera.getView() * M));
                loadMatrix(getUniform("N"), glm::mat3(N));

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

        float magnitude = 0.4f;
};