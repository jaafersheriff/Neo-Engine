#pragma once

#include "Engine.hpp"
#include "ComputeMeshComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

using namespace neo;

class MeshVisShader : public Shader {

public:

    bool wireFrame = true;

    MeshVisShader(const std::string &vert, const std::string &frag) :
        Shader("MeshVis Shader", vert, frag)
    {}

    virtual void render(const CameraComponent &camera) override {
        bind();

        loadUniform("P", camera.getProj());
        loadUniform("V", camera.getView());

        for (auto& model : Engine::getComponentTuples<ComputeMeshComponent, SpatialComponent>()) {
            CHECK_GL(glDisable(GL_CULL_FACE));
            if (wireFrame) {
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            }

            loadUniform("wf", wireFrame);   
            loadUniform("M", model.get<SpatialComponent>()->getModelMatrix());

            /* Bind mesh */
            CHECK_GL(glBindVertexArray(model.get<ComputeMeshComponent>()->mComputeMesh->mVAOID));

            /* DRAW */
            model.get<ComputeMeshComponent>()->mComputeMesh->draw();
        }

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::Checkbox("Wireframe", &wireFrame);
    }
};
