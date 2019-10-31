#pragma once

#pragma once

#include "Engine.hpp"
#include "MetaballsMeshComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

using namespace neo;

class MetaballsComputeShader : public Shader {

public:

    float mRadius = 1.0f;
    int groupSizeWidth = 64;

    MetaballsComputeShader(const std::string &compute) :
        Shader("MetaballsCompute Shader", compute)
    {}

    virtual void render(const CameraComponent &) override {
        bind();

        loadUniform("radius", mRadius);

        if (auto mesh = Engine::getSingleComponent<MetaballsMeshComponent>()) {

            // Bind mesh
            GLuint gIndexBufferBinding = 0;
            CHECK_GL(glBindVertexArray(mesh->mMesh->mVAOID));
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, gIndexBufferBinding, mesh->mMesh->mVertexBufferID));
            CHECK_GL(glEnableVertexAttribArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh->mMesh->mVertexBufferID));
            CHECK_GL(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0));

            // Dispatch 
            // TODO - get max groupsize from GL
            CHECK_GL(glDispatchCompute(mesh->mMesh->mVertexBufferSize / groupSizeWidth, 1, 1));
            CHECK_GL(glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT));

            // Reset bind
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, gIndexBufferBinding, 0));
        }

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::SliderFloat("R", &mRadius, 0.f, 1.f);
        ImGui::SliderInt("GroupSize", &groupSizeWidth, 1, 128);
    }
};
