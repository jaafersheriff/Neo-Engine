#pragma once

#include "ParticleMeshComponent.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"
#include "Renderer/GLObjects/Texture3D.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"

using namespace neo;

namespace Compute {
    class ParticlesComputeShader : public Shader {

    public:

        float timeScale = 100.f;

        ParticlesComputeShader(const std::string& compute) :
            Shader("ParticlesCompute Shader")
        {
            _attachStage(ShaderStage::COMPUTE, compute);
            init();
        }

        virtual void render(const ECS& ecs) override {
            bind();

            if (auto mesh = ecs.getSingleComponent<ParticleMeshComponent>()) {
                auto position = mesh->mMesh->getVBO(VertexType::Position);

                if (auto frameStats = ecs.getSingleComponent<FrameStatsComponent>()) {
                    loadUniform("timestep", frameStats->mDT / 1000.f * timeScale);
                }
                else {
                    loadUniform("timestep", 0.f);
                }

                // Bind mesh
                CHECK_GL(glBindVertexArray(mesh->mMesh->mVAOID));
                CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position.attribArray, position.vboID));

                // Dispatch 
                CHECK_GL(glDispatchCompute(mesh->mNumParticles / Renderer::mDetails.mMaxComputeWorkGroupSize.x, 1, 1));
                CHECK_GL(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));


                // Reset bind
                CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position.attribArray, 0));
            }

            unbind();
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Time scale", &timeScale, 0.f, 1000.f);
        }
    };
}
