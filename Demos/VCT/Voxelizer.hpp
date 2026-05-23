#pragma once

#include "VolumeComponent.hpp"
#include "VoxelizeComponent.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderingComponent/OpaqueComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"

#include "Renderer/RenderingSystems/RenderPass.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

namespace VCT {

	using namespace neo;

	namespace {

		std::pair<ShaderBufferHandle, ShaderBufferHandle> _generateVoxelNodes(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs) {
			TRACY_ZONE();
			auto volumeView = ecs.getSingleView<VolumeComponent, SpatialComponent>();
			if (!volumeView) {
				return {};
			}

			const auto& [_, volume, volumeSpatial] = *volumeView;
			int numVoxels = volume.mDimension * volume.mDimension * volume.mDimension;
			int numNodes = numVoxels * volume.mNodesPerVoxel;

			// Raster path to create voxel nodes
			ShaderBufferHandle voxelNodesHandle;
			ShaderBufferHandle headerPointersHandle;
			{
				TRACY_ZONEN("VoxelNodes");
				struct alignas(16) VoxelNode {
					float albedoR, albedoG, albedoB, abledoA; // TODO - pack
					float normalX, normalY, normalG; // TODO - pack
					int header;
				};

				voxelNodesHandle = resourceManagers.mShaderBufferManager.asyncLoad("VolumeNodes", ShaderBufferLoadDetails{
					static_cast<uint32_t>(numNodes * sizeof(VoxelNode)),
					nullptr
					});
				ShaderBufferHandle atomicCounterHandle = resourceManagers.mShaderBufferManager.asyncLoad("VolumeNodesAtomicCounter", ShaderBufferLoadDetails{
					sizeof(uint32_t),
					nullptr
					});
				std::vector<int> headerPointerData(numVoxels, -1);
				headerPointersHandle = resourceManagers.mShaderBufferManager.asyncLoad("HeaderPointers", ShaderBufferLoadDetails{
					static_cast<uint32_t>(headerPointerData.size() * sizeof(int)),
					reinterpret_cast<const uint8_t*>(headerPointerData.data())
					});


				if (!resourceManagers.mShaderBufferManager.isValid(voxelNodesHandle)
					|| !resourceManagers.mShaderBufferManager.isValid(headerPointersHandle)
					|| !resourceManagers.mShaderBufferManager.isValid(atomicCounterHandle)
					) {
					return {};
				}

				// Resolution changed, destroy everything and try again next frame
				if (resourceManagers.mShaderBufferManager.resolve(voxelNodesHandle).mByteSize != numNodes * sizeof(VoxelNode)) {
					resourceManagers.mShaderBufferManager.discard(voxelNodesHandle);
					resourceManagers.mShaderBufferManager.discard(headerPointersHandle);
					resourceManagers.mShaderBufferManager.discard(atomicCounterHandle);
					return {};
				}

				// Clear the buffers...
				resourceManagers.mShaderBufferManager.transact(voxelNodesHandle, [numNodes](ShaderBuffer& buffer) {
					std::vector<uint8_t> data(numNodes * sizeof(VoxelNode), 0);
					buffer.update(numNodes * sizeof(VoxelNode), data.data());
					});
				resourceManagers.mShaderBufferManager.transact(headerPointersHandle, [numVoxels](ShaderBuffer& buffer) {
					std::vector<int> headerPointerData(numVoxels, -1);
					buffer.update(numVoxels * sizeof(int), reinterpret_cast<const uint8_t*>(headerPointerData.data()));
					});
				resourceManagers.mShaderBufferManager.transact(atomicCounterHandle, [](ShaderBuffer& buffer) {
					uint32_t zero = 0;
					buffer.update(sizeof(uint32_t), reinterpret_cast<const uint8_t*>(&zero));
					});

				auto voxelNodeTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
					"Voxel Nodes Target",
					FramebufferBuilder{}
					.setSize(glm::uvec2(volume.mDimension, volume.mDimension))
					.attach(TextureFormat{ types::texture::Target::Texture2D, types::InternalFormats::RGB8_UNORM }),
					resourceManagers.mTextureManager
				);

				renderPasses.clear(voxelNodeTargetHandle, types::framebuffer::AttachmentBit::Color, glm::vec4(0.f)); // Probably unnecessary

				RenderState renderState;
				renderState.mDepthState = std::nullopt;
				renderState.mCullFace = std::nullopt;
				renderPasses.renderPass(voxelNodeTargetHandle, glm::uvec2(volume.mDimension), renderState,
					[numVoxels, numNodes, voxelNodesHandle, atomicCounterHandle, headerPointersHandle](const ResourceManagers& resourceManagers, const ECS& ecs) {
						TRACY_GPUN("Voxel Nodes");
						const auto& [_, volume, volumeSpatial, volumeCamera, volumeBB] = *ecs.getSingleView<VolumeComponent, SpatialComponent, CameraComponent, BoundingBoxComponent>();

						auto voxelNodeShaderHandle = resourceManagers.mShaderManager.asyncLoad("VoxelNodesShader", ShaderBuilder{}
							.setStage(types::shader::Stage::Vertex, "vct/voxelnodes.vert")
							.setStage(types::shader::Stage::Geometry, "vct/voxelnodes.geom")
							.setStage(types::shader::Stage::Fragment, "vct/voxelnodes.frag")
						);
						if (!resourceManagers.mShaderManager.isValid(voxelNodeShaderHandle)) {
							return;
						}

						auto voxelNodeShader = resourceManagers.mShaderManager.resolveDefines(voxelNodeShaderHandle, {});
						voxelNodeShader.bind();

						glm::vec3 volumeWorldMin = glm::vec3(volumeSpatial.getModelMatrix() * glm::vec4(volumeBB.mMin, 1.0));
						glm::vec3 volumeWorldMax = glm::vec3(volumeSpatial.getModelMatrix() * glm::vec4(volumeBB.mMax, 1.0));

						voxelNodeShader.bindUniform("volumeMin", glm::min(volumeWorldMin, volumeWorldMax));
						voxelNodeShader.bindUniform("volumeMax", glm::max(volumeWorldMin, volumeWorldMax));
						voxelNodeShader.bindUniform("volumeDimension", volume.mDimension);
						voxelNodeShader.bindUniform("numVoxels", numVoxels);
						voxelNodeShader.bindUniform("numNodes", numNodes);
						voxelNodeShader.bindUniform("P", volumeCamera.getProj());
						voxelNodeShader.bindUniform("V", volumeSpatial.getView());
						auto bufferBarrier = voxelNodeShader.bindShaderBuffer("VoxelNodes", resourceManagers.mShaderBufferManager.resolve(voxelNodesHandle), types::shader::Access::ReadWrite);
						auto locksBarrier = voxelNodeShader.bindShaderBuffer("HeaderPointers", resourceManagers.mShaderBufferManager.resolve(headerPointersHandle), types::shader::Access::ReadWrite);
						auto counterBarrier = voxelNodeShader.bindShaderBuffer("NodeCounter", resourceManagers.mShaderBufferManager.resolve(atomicCounterHandle), types::shader::Access::ReadWrite);

						const auto& meshView = ecs.getView<
							const VoxelizeComponent,
							const OpaqueComponent, // Only want to voxelize opaque objects for now
							const MeshComponent,
							const MaterialComponent,
							const SpatialComponent>();
						for (auto entity : meshView) {
							if (resourceManagers.mMeshManager.isValid(ecs.cGetComponent<MeshComponent>(entity)->mMeshHandle)) {
								auto& mesh = resourceManagers.mMeshManager.resolve(ecs.cGetComponent<MeshComponent>(entity)->mMeshHandle);
								const auto& material = ecs.cGetComponent<MaterialComponent>(entity);

								auto meshSpatial = ecs.cGetComponent<SpatialComponent>(entity);
								voxelNodeShader.bindUniform("M", meshSpatial->getModelMatrix());
								voxelNodeShader.bindUniform("N", meshSpatial->getNormalMatrix());
								voxelNodeShader.bindUniform("albedo", glm::vec3(material->mAlbedoColor) + material->mEmissiveFactor);

								mesh.draw();
							}
						}
					}, "Generate VoxelNodes");
			}

			return std::make_pair(headerPointersHandle, voxelNodesHandle);
		}
	}

	std::pair<ShaderBufferHandle, ShaderBufferHandle> voxelize(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs) {
		TRACY_ZONE();
		return _generateVoxelNodes(renderPasses, resourceManagers, ecs);
	}

	void debugVoxelNodes(const ResourceManagers& resourceManagers, const ECS& ecs, ShaderBufferHandle headerBuffer, ShaderBufferHandle voxelNodesBuffer, ECS::Entity cameraEntity) {
		TRACY_ZONE();
		if (!resourceManagers.mShaderBufferManager.isValid(voxelNodesBuffer)
		 || !resourceManagers.mShaderBufferManager.isValid(headerBuffer)) {
			return;
		}

		auto volumeView = ecs.getSingleView<VolumeComponent, SpatialComponent, BoundingBoxComponent>();
		if (!volumeView) {
			return;
		}
		const auto& [_, volume, volumeSpatial, volumeBB] = *volumeView;

		auto voxelNodeDebugShaderHandle = resourceManagers.mShaderManager.asyncLoad("VoxelNodesDebugShader", ShaderBuilder{}
			.setStage(types::shader::Stage::Vertex, "model.vert")
			.setStage(types::shader::Stage::Fragment, "vct/voxelnodesdebug.frag")
		);
		if (!resourceManagers.mShaderManager.isValid(voxelNodeDebugShaderHandle)) {
			return;
		}

		auto voxelNodeShader = resourceManagers.mShaderManager.resolveDefines(voxelNodeDebugShaderHandle, {});
		voxelNodeShader.bind();

		glm::vec3 volumeWorldMin = glm::vec3(volumeSpatial.getModelMatrix() * glm::vec4(volumeBB.mMin, 1.0));
		glm::vec3 volumeWorldMax = glm::vec3(volumeSpatial.getModelMatrix() * glm::vec4(volumeBB.mMax, 1.0));

		voxelNodeShader.bindUniform("volumeMin", glm::min(volumeWorldMin, volumeWorldMax));
		voxelNodeShader.bindUniform("volumeMax", glm::max(volumeWorldMin, volumeWorldMax));
		voxelNodeShader.bindUniform("volumeDimension", volume.mDimension);
		voxelNodeShader.bindUniform("P", ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj());
		voxelNodeShader.bindUniform("V", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getView());
		voxelNodeShader.bindUniform("cameraPos", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getPosition());
		auto nodesBarrier = voxelNodeShader.bindShaderBuffer("VoxelNodes", resourceManagers.mShaderBufferManager.resolve(voxelNodesBuffer), types::shader::Access::Read);
		auto headerBarrier = voxelNodeShader.bindShaderBuffer("HeaderPointers", resourceManagers.mShaderBufferManager.resolve(headerBuffer), types::shader::Access::Read);

		auto& mesh = resourceManagers.mMeshManager.resolve(MeshHandle("cube"));
		voxelNodeShader.bindUniform("M", volumeSpatial.getModelMatrix());
		mesh.draw();
	}
}