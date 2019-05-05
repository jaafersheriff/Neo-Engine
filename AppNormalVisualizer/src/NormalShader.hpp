#pragma once

#include "NeoEngine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

using namespace neo;

class NormalShader : public Shader {

    public: 
    
        NormalShader(const std::string &vert, const std::string &frag, const std::string &geom) :
            Shader("Normal Shader", vert, frag, geom) 
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            loadUniform("magnitude", magnitude);

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            for (auto& model : NeoEngine::getComponents<MeshComponent>()) {
                /* Bind mesh */
                const Mesh & mesh(model->getMesh());
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                glm::mat4 M = model->getGameObject().getSpatial()->getModelMatrix();
                loadUniform("M", M);
                glm::mat4 N = glm::transpose(glm::inverse(camera.getView() * M));
                loadUniform("N", glm::mat3(N));

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