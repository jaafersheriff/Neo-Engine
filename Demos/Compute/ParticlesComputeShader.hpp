#pragma once

#include "ParticleMeshComponent.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"
#include "Renderer/GLObjects/Texture3D.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"

#include <tracy/TracyOpenGL.hpp>

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
            ZoneScoped;
            TracyGpuZone("ParticlesComputeShader");
            bind();

            if (auto meshView = ecs.cGetComponent<ParticleMeshComponent>()) {
                auto&& [_, mesh] = *meshView;
                auto position = mesh.mMesh->getVBO(VertexType::Position);

                if (auto frameStatsView = ecs.cGetComponent<FrameStatsComponent>()) {
                    auto&& [__, frameStats] = *frameStatsView;
                    loadUniform("timestep", frameStats.mDT / 1000.f * timeScale);
                }
                else {
                    loadUniform("timestep", 0.f);
                }

                // Bind mesh
                glBindVertexArray(mesh.mMesh->mVAOID);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position.attribArray, position.vboID);

                // Dispatch 
                glDispatchCompute(mesh.mNumParticles / ServiceLocator<Renderer>::ref().mDetails.mMaxComputeWorkGroupSize.x, 1, 1);
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


                // Reset bind
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position.attribArray, 0);
            }

            unbind();
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Time scale", &timeScale, 0.f, 1000.f);
        }
    };
}
