#pragma once

#include "Engine/Engine.hpp"
#include "ParticleMeshComponent.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"
#include "Renderer/GLObjects/Texture3D.hpp"

using namespace neo;

class ParticlesComputeShader : public Shader {

public:

    float timeScale = 100.f;

    ParticlesComputeShader(const std::string &compute) :
        Shader("ParticlesCompute Shader")
    {
        _attachStage(ShaderStage::COMPUTE, compute);
        init();
    }

    virtual void render() override {
        bind();

        if (auto mesh = Engine::getSingleComponent<ParticleMeshComponent>()) {
            auto position = mesh->mMesh->getVBO(VertexType::Position);

            if (auto frameStats = Engine::getSingleComponent<FrameStatsComponent>()) {
                loadUniform("timestep", frameStats->mDT / 1000.f * timeScale);
            }
            else {
                loadUniform("timestep", 0.f);
            }

            // Bind mesh
            CHECK_GL(glBindVertexArray(mesh->mMesh->mVAOID));
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position.attribArray, position.vboID));

            // Dispatch 
            CHECK_GL(glDispatchCompute(mesh->mNumParticles / Renderer::NEO_MAX_COMPUTE_GROUP_SIZE.x, 1, 1));
            CHECK_GL(glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT ));


            // Reset bind
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position.attribArray, 0));
        }

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::SliderFloat("Time scale", &timeScale, 0.f, 1000.f);
    }
};
