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
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "ResourceManager/ResourceManagers.hpp"

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

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "Compute";
		config.clearColor = { 0.f, 0.f, 0.f };
		return config;
	}

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {

		/* Game objects */
		Camera camera(ecs, 45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
		ecs.addComponent<MainCameraComponent>(camera.mEntity);

		// Create mesh
		{
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, "Particles");
			ecs.addComponent<ParticleMeshComponent>(entity, resourceManagers.mMeshManager);
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 0.0f, 0.f));
		}

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>();
	}

	void Demo::update(ECS& ecs, ResourceManagers& resourceManagers) {
		if (auto meshView = ecs.getComponent<ParticleMeshComponent>()) {
			TRACY_GPUN("Update Particles");
			auto&& [_, meshComponent] = *meshView;

			// Update base verts
			if (meshComponent.isDirty) {
				std::vector<float> positions;
				positions.resize(meshComponent.mNumParticles * 4);
				for (int i = 0; i < meshComponent.mNumParticles; i++) {
					glm::vec3 pos = glm::normalize(util::genRandomVec3(-1.f, 1.f));
					positions[i * 4 + 0] = pos.x;
					positions[i * 4 + 1] = pos.y;
					positions[i * 4 + 2] = pos.z;
					positions[i * 4 + 3] = 1.f; // TODO - this is useless and costs perf. Get rid of it
				}

				auto& mesh = resourceManagers.mMeshManager.resolve(meshComponent.mMeshHandle);
				mesh.updateVertexBuffer(
					types::mesh::VertexType::Position,
					static_cast<uint32_t>(positions.size()),
					static_cast<uint32_t>(positions.size() * sizeof(float)),
					reinterpret_cast<uint8_t*>(positions.data())
				);

				meshComponent.isDirty = false;
			}
		}
	}

	void Demo::render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) {
		TRACY_GPUN("Compute::render")
		backbuffer.bind();
		backbuffer.clear(glm::vec4(getConfig().clearColor, 1.0), types::framebuffer::ClearFlagBits::Color | types::framebuffer::ClearFlagBits::Depth);

		// Update the mesh
		if (auto meshView = ecs.cGetComponent<ParticleMeshComponent>()) {
			TRACY_GPUN("Update Particles");
			auto&& [_, meshComponent] = *meshView;

			auto particlesComputeShaderHandle = resourceManagers.mShaderManager.asyncLoad("ParticlesCompute", SourceShader::ConstructionArgs{
				{ ShaderStage::COMPUTE, "compute/particles.compute" }
			});

			auto& particlesComputeShader = resourceManagers.mShaderManager.resolveDefines(particlesComputeShaderHandle, {});
			particlesComputeShader.bind();

			float timeStep = 0.f;
			if (auto frameStatsView = ecs.cGetComponent<FrameStatsComponent>()) {
				auto&& [__, frameStats] = *frameStatsView;
				timeStep = frameStats.mDT / 1000.f * meshComponent.timeScale;
			}
			particlesComputeShader.bindUniform("timestep", timeStep);

			// Bind mesh
			auto& mesh = resourceManagers.mMeshManager.resolve(meshComponent.mMeshHandle);
			auto& position = mesh.getVBO(types::mesh::VertexType::Position);
			glBindVertexArray(mesh.mVAOID);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position.attribArray, position.vboID);

			// Dispatch 
			glDispatchCompute(meshComponent.mNumParticles / ServiceLocator<Renderer>::ref().mDetails.mMaxComputeWorkGroupSize.x, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			// Reset bind
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position.attribArray, 0);
		}

		// Draw the mesh
		{
			TRACY_GPUN("Draw Particles");
			auto particlesVisShaderHandle = resourceManagers.mShaderManager.asyncLoad("ParticleVis", SourceShader::ConstructionArgs{
				{ ShaderStage::VERTEX,   "compute/particles.vert" },
				{ ShaderStage::GEOMETRY, "compute/particles.geom" },
				{ ShaderStage::FRAGMENT, "compute/particles.frag" },
			});

			auto& particlesVisShader = resourceManagers.mShaderManager.resolveDefines(particlesVisShaderHandle, {});
			particlesVisShader.bind();

			if (auto cameraView = ecs.getSingleView<MainCameraComponent, PerspectiveCameraComponent, SpatialComponent>()) {
				auto&& [_, __, camera, camSpatial] = *cameraView;
				particlesVisShader.bindUniform("P", camera.getProj());
				particlesVisShader.bindUniform("V", camSpatial.getView());
			}
			particlesVisShader.bindUniform("spriteSize", mSpriteSize);
			particlesVisShader.bindUniform("spriteColor", mSpriteColor);

			if (auto meshView = ecs.getSingleView<ParticleMeshComponent, SpatialComponent>()) {
				auto&& [_, meshComponent, spatial] = *meshView;
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);

				particlesVisShader.bindUniform("M", spatial.getModelMatrix());

				/* DRAW */
				resourceManagers.mMeshManager.resolve(meshComponent.mMeshHandle).draw();
			}
		}
	}

	void Demo::imGuiEditor(ECS& ecs) {
		NEO_UNUSED(ecs);
		ImGui::SliderFloat("Sprite size", &mSpriteSize, 0.1f, 2.f);
		ImGui::ColorEdit3("Sprite color", &mSpriteColor[0]);
	}
}
