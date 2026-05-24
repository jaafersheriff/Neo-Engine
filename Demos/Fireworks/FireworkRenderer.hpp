#pragma once

#include "ECS/ECS.hpp"

#include "Util/Profiler.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

namespace Fireworks {
	inline void tickParticles(const ResourceManagers& resourceManagers, const ECS& ecs) {
		TRACY_GPU();

		float timeStep = 0.f;
		if (auto frameStatsView = ecs.cGetComponent<FrameStatsComponent>()) {
			auto&& [__, frameStats] = *frameStatsView;
			timeStep = frameStats.mDT;
		}

		auto fireworksComputeShaderHandle = resourceManagers.mShaderManager.asyncLoad("FireworksCompute", ShaderBuilder{}
			.setStage(types::shader::Stage::Compute, "firework/firework_tick.compute")
		);
		if (!resourceManagers.mShaderManager.isValid(fireworksComputeShaderHandle)) {
			return;
		}

		for (const auto& [_, spatial, firework, light] : ecs.getView<SpatialComponent, FireworkComponent, LightComponent>().each()) {

			if (!resourceManagers.mMeshManager.isValid(firework.mBuffer)) {
				continue;
			}
			
			auto& fireworksComputeShader = resourceManagers.mShaderManager.resolveDefines(fireworksComputeShaderHandle, {});
			fireworksComputeShader.bind();

			fireworksComputeShader.bindUniform("random", glm::vec4(util::genRandomVec3(), util::genRandom()));

			fireworksComputeShader.bindUniform("timestep", timeStep);
			fireworksComputeShader.bindUniform("lightPos", spatial.getPosition());

			fireworksComputeShader.bindUniform("infinite", firework.mParameters.mInfinite ? 1 : 0);
			fireworksComputeShader.bindUniform("baseSpeed", firework.mParameters.mBaseSpeed);
			fireworksComputeShader.bindUniform("velocityDecay", 1.f - firework.mParameters.mVelocityDecay);
			fireworksComputeShader.bindUniform("gravity", firework.mParameters.mGravity);
			fireworksComputeShader.bindUniform("minIntensity", firework.mParameters.mMinIntensity);

			fireworksComputeShader.bindUniform("parentIntensity", light.mIntensity);
			fireworksComputeShader.bindUniform("parentSpeed", firework.mParameters.mParentSpeed);
			fireworksComputeShader.bindUniform("parentIntensityDecay", 1.f - firework.mParameters.mParentIntensityDecay);

			fireworksComputeShader.bindUniform("numChildren", firework.mParameters.mChildren);
			fireworksComputeShader.bindUniform("childPosOffset", firework.mParameters.mChildPositionOffset);
			fireworksComputeShader.bindUniform("childIntensity", firework.mParameters.mChildIntensity);
			fireworksComputeShader.bindUniform("childVelocityBias", firework.mParameters.mChildVelocityBias);
			fireworksComputeShader.bindUniform("childIntensityDecay", 1.f - firework.mParameters.mChildIntensityDecay);

			// Bind mesh vertex buffers as shader storage buffers
			auto& mesh = resourceManagers.mMeshManager.resolve(firework.mBuffer);
			fireworksComputeShader.bindShaderBuffer("BufferA", mesh, types::mesh::VertexType::Position, types::shader::Access::ReadWrite);
			fireworksComputeShader.bindShaderBuffer("BufferB", mesh, types::mesh::VertexType::Normal, types::shader::Access::ReadWrite);

			// Dispatch 
			fireworksComputeShader.dispatch({ std::ceil(firework.mCount / 16), 1, 1 });
			ShaderBarrier barrier(types::shader::Barrier::StorageBuffer);
		}
	}

	inline void drawParticles(const ResourceManagers& resourceManagers, const ECS& ecs) {
		TRACY_GPU();
		auto fireworksVisShaderHandle = resourceManagers.mShaderManager.asyncLoad("FireworkDraw", ShaderBuilder{}
			.setStage(types::shader::Stage::Vertex,   "firework/firework.vert")
			.setStage(types::shader::Stage::Geometry, "firework/firework.geom")
			.setStage(types::shader::Stage::Fragment, "firework/firework.frag")
		);

		if (!resourceManagers.mShaderManager.isValid(fireworksVisShaderHandle)) {
			return;
		}

		auto& fireworksVisShader = resourceManagers.mShaderManager.resolveDefines(fireworksVisShaderHandle, {});
		fireworksVisShader.bind();

		if (auto cameraView = ecs.getSingleView<MainCameraComponent, CameraComponent, SpatialComponent>()) {
			auto&& [_, __, camera, camSpatial] = *cameraView;
			fireworksVisShader.bindUniform("P", camera.getProj());
			fireworksVisShader.bindUniform("V", camSpatial.getView());
		}

		for (const auto& [_, spatial, firework, light] : ecs.getView<SpatialComponent, FireworkComponent, LightComponent>().each()) {
			fireworksVisShader.bindUniform("parentColor", light.mColor);
			fireworksVisShader.bindUniform("parentLength", firework.mParameters.mParentLength);

			fireworksVisShader.bindUniform("childColor", firework.mParameters.mChildColor);
			fireworksVisShader.bindUniform("childColorBias", firework.mParameters.mChildColorBias);
			fireworksVisShader.bindUniform("childLength", firework.mParameters.mChildLength);

			fireworksVisShader.bindUniform("M", spatial.getModelMatrix());

			/* DRAW */
			if (resourceManagers.mMeshManager.isValid(firework.mBuffer)) {
				resourceManagers.mMeshManager.resolve(firework.mBuffer).draw();
			}
		}
	}
}