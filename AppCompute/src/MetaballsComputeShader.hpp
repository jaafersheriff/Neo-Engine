#pragma once

#pragma once

#include "Engine.hpp"
#include "MetaballsMeshComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

using namespace neo;

class MetaballsComputeShader : public Shader {

public:

    float radius = 1.0f;
    bool autoUpdate = false;
    int groupSizeWidth = 64;

    MetaballsComputeShader(const std::string &compute) :
        Shader("MetaballsCompute Shader", compute)
    {}

    virtual void render(const CameraComponent &) override {
        bind();

        loadUniform("radius", autoUpdate ? glm::sin(Util::getRunTime()) : radius);

        if (auto mesh = Engine::getSingleComponent<MetaballsMeshComponent>()) {

            // Bind the VBO to the SSBO, that is filled in the compute shader.
            // gIndexBufferBinding is equal to 0. This is the same as the compute shader binding.
            GLuint gIndexBufferBinding = 0;
            CHECK_GL(glBindVertexArray(mesh->mMesh->mVAOID));
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, gIndexBufferBinding, mesh->mMesh->mVertexBufferID));
            CHECK_GL(glEnableVertexAttribArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh->mMesh->mVertexBufferID));
            CHECK_GL(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0));

            // Submit a job for the compute shader execution.
            // GROUP_SIZE = 64
            // NUM_VERTS = 256
            // As the result the function is called with the following parameters:
            // glDispatchCompute(4, 1, 1)
            CHECK_GL(glDispatchCompute(mesh->mMesh->mVertexBufferSize / groupSizeWidth, 1, 1));
            CHECK_GL(glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT));

            // Unbind the SSBO buffer.
            // gIndexBufferBinding is equal to 0. This is the same as the compute shader binding.
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, gIndexBufferBinding, 0));
        }

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::Checkbox("Auto", &autoUpdate);
        if (!autoUpdate) {
            ImGui::SliderFloat("R", &radius, 0.f, 1.f);
        }
        ImGui::SliderInt("GroupSize", &groupSizeWidth, 1, 128);
    }
};
