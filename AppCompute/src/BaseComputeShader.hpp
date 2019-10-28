#pragma once

#pragma once

#include "Engine.hpp"
#include "ComputeMeshComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

using namespace neo;

class BaseComputeShader : public Shader {

public:

    float radius = 0.1f;
    int groupSizeWidth = 64;

    BaseComputeShader(const std::string &compute) :
        Shader("BaseCompute Shader", compute)
    {}

    virtual void render(const CameraComponent &) override {
        bind();

        loadUniform("radius", radius);

        auto mesh = Engine::getSingleComponent<ComputeMeshComponent>();

        // Bind the VBO to the SSBO, that is filled in the compute shader.
        // gIndexBufferBinding is equal to 0. This is the same as the compute shader binding.
        GLuint gIndexBufferBinding = 0;
        glBindVertexArray(mesh->mComputeMesh->mVAOID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, gIndexBufferBinding, mesh->mComputeMesh->mVertexBufferID);

        // Submit a job for the compute shader execution.
        // GROUP_SIZE = 64
        // NUM_VERTS = 256
        // As the result the function is called with the following parameters:
        // glDispatchCompute(4, 1, 1)
        glDispatchCompute(mesh->mNumVerts / groupSizeWidth, 1, 1);

        // Unbind the SSBO buffer.
        // gIndexBufferBinding is equal to 0. This is the same as the compute shader binding.
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, gIndexBufferBinding, 0);

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::SliderFloat("R", &radius, 0.f, 1.f);
    }
};
