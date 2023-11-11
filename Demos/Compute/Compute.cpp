#include "Compute.hpp"
#include "Engine/Engine.hpp"

#include "ParticleMeshComponent.hpp"

#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"

using namespace neo;

/* Game object definitions */
namespace Compute {
    struct Camera {
        ECS::Entity mEntity;
        Camera(ECS& ecs, float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
            mEntity = ecs.createEntity();
            ecs.addComponent<SpatialComponent>(mEntity, pos, glm::vec3(1.f));
            ecs.addComponent<PerspectiveCameraComponent>(mEntity, near, far, fov);
            ecs.addComponent<CameraControllerComponent>(mEntity, ls, ms);
        }
    };

    struct Light {
        Light(ECS& ecs, glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
            auto entity = ecs.createEntity();
            ecs.addComponent<SpatialComponent>(entity, pos);
            ecs.addComponent<LightComponent>(entity, col, att);
        }
    };

    IDemo::Config Demo::getConfig() const {
        IDemo::Config config;
        config.name = "Compute";
        config.clearColor = { 0.f, 0.f, 0.f };
        return config;
    }

    void Demo::init(ECS& ecs) {

        /* Game objects */
        Camera camera(ecs, 45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
        ecs.addComponent<MainCameraComponent>(camera.mEntity);

        Light(ecs, glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

        // Create mesh
        {
            auto entity = ecs.createEntity();
            ecs.addComponent<TagComponent>(entity, "Particles");
            ecs.addComponent<ParticleMeshComponent>(entity);
            ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 0.0f, 0.f));
        }

        /* Systems - order matters! */
        ecs.addSystem<CameraControllerSystem>();
    }

    void Demo::render(const ECS& ecs, Framebuffer& backbuffer) {
        backbuffer.bind();
        backbuffer.clear(glm::vec4(getConfig().clearColor, 1.0), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update the mesh
        if (auto meshView = ecs.cGetComponent<ParticleMeshComponent>()) {
            auto&& [_, mesh] = *meshView;
            auto& particlesCompute = Library::createShaderSource("ParticlesCompute", NewShader::ConstructionArgs{
                { ShaderStage::COMPUTE, "compute/particles.compute" }
            })->getResolvedInstance({});

            particlesCompute.bind();

            if (auto frameStatsView = ecs.cGetComponent<FrameStatsComponent>()) {
                auto&& [__, frameStats] = *frameStatsView;
                particlesCompute.bindUniform("timestep", frameStats.mDT / 1000.f * mesh.timeScale);
            }
            else {
                particlesCompute.bindUniform("timestep", 0.f);
            }

            // Bind mesh
            auto position = mesh.mMesh->getVBO(VertexType::Position);
            glBindVertexArray(mesh.mMesh->mVAOID);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position.attribArray, position.vboID);

            // Dispatch 
            glDispatchCompute(mesh.mNumParticles / ServiceLocator<Renderer>::ref().mDetails.mMaxComputeWorkGroupSize.x, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            // Reset bind
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position.attribArray, 0);
        }

        // Draw the mesh
        {
            auto& particlesVis = Library::createShaderSource("ParticleVis", NewShader::ConstructionArgs{
                { ShaderStage::VERTEX,   "compute/particles.vert" },
                { ShaderStage::GEOMETRY, "compute/particles.geom" },
                { ShaderStage::FRAGMENT, "compute/particles.frag" },
            })->getResolvedInstance({});
            particlesVis.bind();

            if (auto cameraView = ecs.getSingleView<MainCameraComponent, PerspectiveCameraComponent, SpatialComponent>()) {
                auto&& [_, __, camera, camSpatial] = *cameraView;
                particlesVis.bindUniform("P", camera.getProj());
                particlesVis.bindUniform("V", camSpatial.getView());
            }
            particlesVis.bindUniform("spriteSize", mSpriteSize);
            particlesVis.bindUniform("spriteColor", mSpriteColor);

            if (auto meshView = ecs.getSingleView<ParticleMeshComponent, SpatialComponent>()) {
                auto&& [_, mesh, spatial] = *meshView;
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);

                particlesVis.bindUniform("M", spatial.getModelMatrix());

                /* DRAW */
                mesh.mMesh->draw();
            }
        }
    }

    void Demo::imGuiEditor(ECS& ecs) {
        NEO_UNUSED(ecs);
        ImGui::SliderFloat("Sprite size", &mSpriteSize, 0.1f, 2.f);
        ImGui::ColorEdit3("Sprite color", &mSpriteColor[0]);
    }
}