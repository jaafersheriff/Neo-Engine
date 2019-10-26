#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

using namespace neo;

class NormalShader : public Shader {

    public: 
        float magnitude = 0.4f;
    
        NormalShader(const std::string &vert, const std::string &frag, const std::string &geom) :
            Shader("Normal Shader", vert, frag, geom) 
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            loadUniform("magnitude", magnitude);

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            for (auto& renderable : Engine::getComponentTuples<MeshComponent, SpatialComponent>()) {
                /* Bind mesh */
                const Mesh & mesh(renderable.get<MeshComponent>()->getMesh());
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                glm::mat4 M = renderable.get<SpatialComponent>()->getModelMatrix();
                loadUniform("M", M);
                glm::mat4 N = glm::transpose(glm::inverse(camera.getView() * M));
                loadUniform("N", glm::mat3(N));

                /* DRAW */
                mesh.draw();
            }

            unbind();
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Magnitude", &magnitude, 0.f, 1.f);
        }

};