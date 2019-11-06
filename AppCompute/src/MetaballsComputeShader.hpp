#pragma once

#pragma once

#include "Engine.hpp"
#include "MetaballsMeshComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

using namespace neo;

class MetaballsComputeShader : public Shader {

public:

    int groupSizeWidth = 64;

    MetaballsComputeShader(const std::string &compute) :
        Shader("MetaballsCompute Shader", compute)
    {}

    virtual void render(const CameraComponent &) override {
        bind();

        if (auto mesh = Engine::getSingleComponent<MetaballsMeshComponent>()) {

            // Bind mesh
            CHECK_GL(glBindVertexArray(mesh->mMesh->mVAOID));
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh->mMesh->mVertexBufferID));
            // CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh->mMesh->mNormalBufferID));
            // CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mesh->mMesh->mTexBufferID));
            // CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mesh->mMesh->mElementBufferID));

            // Dispatch 
            // TODO - get max groupsize from GL
            CHECK_GL(glDispatchCompute(mesh->mNumVerts / groupSizeWidth, 1, 1));
            CHECK_GL(glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT));

            // Reset bind
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));
            // CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0));
            // CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0));
            // CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0));
        }

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::SliderInt("GroupSize", &groupSizeWidth, 1, 128);
    }
};
