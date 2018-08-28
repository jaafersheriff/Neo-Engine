#pragma once

#include "NeoEngine.hpp"

#include "Shader/Shader.hpp"
#include "GLHelper/GlHelper.hpp"

using namespace neo;

class NormalShader : public Shader {

    public: 
    
        NormalShader(RenderSystem &r, const std::string &vert, const std::string &frag, const std::string &geom) :
            Shader("Normal Shader", r.APP_SHADER_DIR, vert, frag, geom) 
        {}

        virtual void render(const RenderSystem &renderSystem, const CameraComponent &camera) override {
            bind();

            loadUniform("magnitude", magnitude);

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            for (auto model : renderSystem.getRenderables<NormalShader, RenderableComponent>()) {
                glm::mat4 M = model->getGameObject().getSpatial()->getModelMatrix();
                loadUniform("M", M);
                glm::mat4 N = glm::transpose(glm::inverse(camera.getView() * M));
                loadUniform("N", glm::mat3(N));

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
            unbind();
        }

        float magnitude = 0.4f;
};