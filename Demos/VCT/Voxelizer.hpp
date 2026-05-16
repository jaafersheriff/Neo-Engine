#pragma once

#include "VolumeComponent.hpp"
#include "VoxelizeComponent.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderingComponent/OpaqueComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"

#include "Renderer/RenderingSystems/RenderPass.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

namespace VCT {

	using namespace neo;

	void voxelize(RenderPasses& renderPasses) {

		renderPasses.computePass([](const ResourceManagers& resourceManagers, const ECS& ecs) {
			auto volumeView = ecs.getSingleView<VolumeComponent, SpatialComponent>();
			if (!volumeView) {
				return;
			}

			const auto& [_, volume, volumeSpatial] = *volumeView;

			if (!resourceManagers.mShaderBufferManager.isValid(volume.mBufferHandle)) {
				return;
			}

			auto voxelizeComputeShaderHandle = resourceManagers.mShaderManager.asyncLoad("VoxelizeCompute", ShaderBuilder{}
				.setStage(types::shader::Stage::Compute, "vct/voxelize.compute")
			);
			if (!resourceManagers.mShaderManager.isValid(voxelizeComputeShaderHandle)) {
				return;
			}

			MakeDefine(NoIndices);
			ShaderDefines noIndices;
			noIndices.set(NoIndices);

			auto& positionsOnlyShader = resourceManagers.mShaderManager.resolveDefines(voxelizeComputeShaderHandle, noIndices);
			auto& indicesShader = resourceManagers.mShaderManager.resolveDefines(voxelizeComputeShaderHandle, {});

			const auto& meshView = ecs.getView<
				const VoxelizeComponent,
				const OpaqueComponent, // Only want to voxelize opaque objects for now
				const MeshComponent,
				const MaterialComponent,
				const SpatialComponent>();
			for (auto entity : meshView) {

				// Bind mesh vertex buffers as shader storage buffers
				auto& mesh = resourceManagers.mMeshManager.resolve(ecs.cGetComponent<MeshComponent>(entity)->mMeshHandle);
				if (mesh.mPrimitiveType != types::mesh::Primitive::Triangles) {
					NEO_LOG_E("Mesh is not made of triangles and cannot be voxelized!");
					continue;
				}
				auto& shader = mesh.hasIBO() ? indicesShader : positionsOnlyShader;

				auto volumeBarrier = shader.bindShaderBuffer("Volume", resourceManagers.mShaderBufferManager.resolve(volume.mBufferHandle), types::shader::Access::ReadWrite);
				shader.bindUniform("volumeWorldMatrix", volumeSpatial.getModelMatrix());
				shader.bindUniform("meshWorldMatrix", ecs.cGetComponent<SpatialComponent>(entity)->getModelMatrix());

				auto posBarrier = shader.bindShaderBuffer("Positions", mesh, types::mesh::VertexType::Position, types::shader::Access::Read);
				const int threadGroupSize = 64;
				if (mesh.hasIBO()) {
					int triangleCount = mesh.getIBO().elementCount / 3;
					shader.bindUniform("triangleCount", triangleCount);

					auto indBarrier = shader.bindMeshIndices("Indices", mesh, types::shader::Access::Read);
					shader.bind();
					shader.dispatch({ triangleCount + threadGroupSize - 1 / threadGroupSize, 1, 1 });
				}
				else {
					auto& positionBuffer = mesh.getVBO(types::mesh::VertexType::Position);
					if (positionBuffer.components != 3) {
						NEO_LOG_E("Position buffer isn't in 3D space");
						continue;
					}
					int triangleCount = positionBuffer.elementCount / positionBuffer.components / 3;
					shader.bindUniform("triangleCount", triangleCount);
					shader.bind();
					shader.dispatch({ triangleCount + threadGroupSize - 1 / threadGroupSize, 1, 1 });
				}
			}
		});
	}
}