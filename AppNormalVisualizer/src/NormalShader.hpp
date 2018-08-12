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

        virtual void render(float dt, const RenderSystem &renderSystem) override {
            bind();

            loadFloat(getUniform("magnitude"), magnitude);

            /* Load PV */
            auto cameras = NeoEngine::getComponents<CameraComponent>();
            if (cameras.size()) {
                loadMatrix(getUniform("P"), cameras.at(0)->getProj());
                loadMatrix(getUniform("V"), cameras.at(0)->getView());
            }

            for (auto model : renderSystem.getRenderables<NormalShader, RenderableComponent>()) {
                glm::mat4 M = model->getGameObject().getSpatial()->getModelMatrix();
                loadMatrix(getUniform("M"), M);
                glm::mat4 N = glm::transpose(glm::inverse(cameras.at(0)->getView() * M));
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