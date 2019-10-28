#pragma once

#include "Engine.hpp"
#include "ComputeMeshComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

using namespace neo;

class MeshVisShader : public Shader {

public:

    MeshVisShader(const std::string &vert, const std::string &frag) :
        Shader("MeshVis Shader", vert, frag)
    {}

    virtual void render(const CameraComponent &camera) override {
        bind();

        for (auto& model : Engine::getComponents<ComputeMeshComponent>()) {
            /* Bind mesh */
            CHECK_GL(glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT));
            CHECK_GL(glBindVertexArray(model->mComputeMesh->mVAOID));

            /* DRAW */
            model->mComputeMesh->draw();
        }

        unbind();
    }
};
