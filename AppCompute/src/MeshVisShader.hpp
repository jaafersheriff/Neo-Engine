#pragma once

#include "Engine.hpp"
#include "ComputeMeshComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

using namespace neo;

class MeshVisShader : public Shader {

public:

    bool wireFrame = false;

    MeshVisShader(const std::string &vert, const std::string &frag) :
        Shader("MeshVis Shader", vert, frag)
    {}

    virtual void render(const CameraComponent &camera) override {
        bind();

        loadUniform("P", camera.getProj());
        loadUniform("V", camera.getView());

        for (auto& model : Engine::getComponents<ComputeMeshComponent>()) {
            CHECK_GL(glDisable(GL_CULL_FACE));
            if (wireFrame) {
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            }

            loadUniform("wf", wireFrame);   

            glEnableVertexAttribArray(0);
            /* Bind mesh */
            glBindVertexArray(model->mComputeMesh->mVAOID);
            CHECK_GL(glBindVertexArray(model->mComputeMesh->mVAOID));

            /* DRAW */
            model->mComputeMesh->draw();
        }

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::Checkbox("Wireframe", &wireFrame);
    }
};
