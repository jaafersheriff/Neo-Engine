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
			ShaderBufferHandle atomicCounterHandle;
			{
				TRACY_ZONEN("VoxelNodes");
				struct alignas(16) VoxelNode {
					uint32_t albedo;
					uint32_t normal; // w is unused hmmm
					uint32_t emissive;
					int32_t header;
				};

				voxelNodesHandle = resourceManagers.mShaderBufferManager.asyncLoad("VolumeNodes", ShaderBufferLoadDetails{
					static_cast<uint32_t>(numNodes * sizeof(VoxelNode)),
					nullptr
					});
				atomicCounterHandle = resourceManagers.mShaderBufferManager.asyncLoad("VolumeNodesAtomicCounter", ShaderBufferLoadDetails{
					sizeof(uint32_t),
					nullptr
					});
				headerPointersHandle = resourceManagers.mShaderBufferManager.asyncLoad("HeaderPointers", ShaderBufferLoadDetails{
					static_cast<uint32_t>(numVoxels * sizeof(int)),
					nullptr
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
					buffer.clear(numNodes * sizeof(VoxelNode), 0);
					});
				resourceManagers.mShaderBufferManager.transact(headerPointersHandle, [numVoxels](ShaderBuffer& buffer) {
					buffer.clear(numVoxels * sizeof(int), -1);
					});
				resourceManagers.mShaderBufferManager.transact(atomicCounterHandle, [](ShaderBuffer& buffer) {
					uint32_t zero = 0;
					buffer.update(sizeof(uint32_t), reinterpret_cast<const uint8_t*>(&zero));
					});
			}

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

					glm::vec3 volumeWorldMin = glm::vec3(volumeSpatial.getModelMatrix() * glm::vec4(volumeBB.mMin, 1.0));
					glm::vec3 volumeWorldMax = glm::vec3(volumeSpatial.getModelMatrix() * glm::vec4(volumeBB.mMax, 1.0));

					ShaderDefines albedoDefines;
					MakeDefine(ALBEDO_MAP);
					albedoDefines.set(ALBEDO_MAP);

					const auto& meshView = ecs.getView<
						const VoxelizeComponent,
						const MeshComponent,
						const MaterialComponent,
						const SpatialComponent>();
					for (auto entity : meshView) {

						auto meshSpatial = ecs.cGetComponent<SpatialComponent>(entity);
						if (auto meshBB = ecs.cGetComponent<BoundingBoxComponent>(entity)) {
							if (!volumeBB.intersect(volumeSpatial.getModelMatrix(), *meshBB, meshSpatial->getModelMatrix())) {
								continue;
							}
						}

						if (!resourceManagers.mMeshManager.isValid(ecs.cGetComponent<MeshComponent>(entity)->mMeshHandle)) {
							continue;
						}

						const auto& material = ecs.cGetComponent<MaterialComponent>(entity);

						auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(voxelNodeShaderHandle, resourceManagers.mTextureManager.isValid(material->mAlbedoMap) ? albedoDefines : ShaderDefines{});

						resolvedShader.bindShaderBuffer("NodeCounter", resourceManagers.mShaderBufferManager.resolve(atomicCounterHandle), types::shader::Access::ReadWrite);
						resolvedShader.bindShaderBuffer("VoxelNodes", resourceManagers.mShaderBufferManager.resolve(voxelNodesHandle), types::shader::Access::ReadWrite);
						resolvedShader.bindShaderBuffer("HeaderPointers", resourceManagers.mShaderBufferManager.resolve(headerPointersHandle), types::shader::Access::ReadWrite);

						resolvedShader.bindUniform("volumeMin", glm::min(volumeWorldMin, volumeWorldMax));
						resolvedShader.bindUniform("volumeMax", glm::max(volumeWorldMin, volumeWorldMax));
						resolvedShader.bindUniform("volumeDimension", volume.mDimension);
						resolvedShader.bindUniform("numVoxels", numVoxels);
						resolvedShader.bindUniform("numNodes", numNodes);
						resolvedShader.bindUniform("P", volumeCamera.getProj());
						resolvedShader.bindUniform("V", volumeSpatial.getView());

						resolvedShader.bindUniform("M", meshSpatial->getModelMatrix());
						resolvedShader.bindUniform("N", meshSpatial->getNormalMatrix());
						resolvedShader.bindUniform("albedo", material->mAlbedoColor);
						resolvedShader.bindUniform("emissive", material->mEmissiveFactor);
						if (resourceManagers.mTextureManager.isValid(material->mAlbedoMap)) {
							resolvedShader.bindTexture("albedoMap", resourceManagers.mTextureManager.resolve(material->mAlbedoMap));
						}

						resourceManagers.mMeshManager.resolve(ecs.cGetComponent<MeshComponent>(entity)->mMeshHandle).draw();
					}

					ShaderBarrier barrier(types::shader::Barrier::StorageBuffer);
				}, "Generate VoxelNodes");

			return std::make_pair(headerPointersHandle, voxelNodesHandle);
		}

		std::pair<ShaderBufferHandle, ShaderBufferHandle> _generateBrickIDs(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs, const ShaderBufferHandle headerPointersHandle, const ShaderBufferHandle voxelNodesHandle) {
			TRACY_ZONE();

			if (!resourceManagers.mShaderBufferManager.isValid(headerPointersHandle)
				|| !resourceManagers.mShaderBufferManager.isValid(voxelNodesHandle)) {
				return {};
			}
			auto volumeView = ecs.getSingleView<VolumeComponent, SpatialComponent>();
			if (!volumeView) {
				return {};
			}

			const auto& [_, volume, volumeSpatial] = *volumeView;
			int bricksPerAxis = (volume.mDimension + volume.mVoxelsPerBrick - 1) / volume.mVoxelsPerBrick; // ceil(dimension / voxelsPerBrick)
			int numBricks = bricksPerAxis * bricksPerAxis * bricksPerAxis;

			ShaderBufferHandle brickIDsHandle;
			{
				{
					brickIDsHandle = resourceManagers.mShaderBufferManager.asyncLoad("BrickIDs", ShaderBufferLoadDetails{
						static_cast<uint32_t>(numBricks * sizeof(int)),
						nullptr
						});


					if (!resourceManagers.mShaderBufferManager.isValid(brickIDsHandle)) {
						return;
					}

					// Resolution changed, destroy everything and try again next frame
					if (resourceManagers.mShaderBufferManager.resolve(brickIDsHandle).mByteSize != numBricks * sizeof(int)) {
						resourceManagers.mShaderBufferManager.discard(brickIDsHandle);
						return;
					}

					// Clear the buffers...
					resourceManagers.mShaderBufferManager.transact(brickIDsHandle, [numBricks](ShaderBuffer& buffer) {
						buffer.clear(numBricks * sizeof(int), -1);
						});
				}

				// First, walk through each voxel (and its nodes) to determine which bricks are active
				renderPasses.computePass([headerPointersHandle, brickIDsHandle, bricksPerAxis](const ResourceManagers& resourceManagers, const ECS& ecs) {
					TRACY_GPUN("Active Bricks");
					auto activeBrickShaderHandle = resourceManagers.mShaderManager.asyncLoad("ActiveBrickShader", ShaderBuilder{}
						.setStage(types::shader::Stage::Compute, "vct/activebricks.compute")
					);
					if (!resourceManagers.mShaderManager.isValid(activeBrickShaderHandle)) {
						return;
					}

					auto volumeView = ecs.getSingleView<VolumeComponent, SpatialComponent>();
					if (!volumeView) {
						return;
					}

					const auto& [_, volume, volumeSpatial] = *volumeView;

					auto resolvedShader = resourceManagers.mShaderManager.resolveDefines(activeBrickShaderHandle, {});

					resolvedShader.bindShaderBuffer("HeaderPointers", resourceManagers.mShaderBufferManager.resolve(headerPointersHandle), types::shader::Access::Read);
					resolvedShader.bindShaderBuffer("ActiveBricks", resourceManagers.mShaderBufferManager.resolve(brickIDsHandle), types::shader::Access::ReadWrite);
					resolvedShader.bindUniform("volumeDimension", volume.mDimension);
					resolvedShader.bindUniform("brickDimension", volume.mVoxelsPerBrick);
					resolvedShader.bindUniform("bricksPerAxis", bricksPerAxis);

					const int localSize = 8;
					int groupSize = (volume.mDimension + localSize - 1) / localSize;  // ceil(dimension / localSize)
					resolvedShader.dispatch(glm::uvec3(groupSize));
					ShaderBarrier barrier(types::shader::Barrier::StorageBuffer);
					}, "Active Bricks");
			}

			ShaderBufferHandle brickCounterHandle;
			{
				{
					brickCounterHandle = resourceManagers.mShaderBufferManager.asyncLoad("BrickCounter", ShaderBufferLoadDetails{
						sizeof(uint32_t),
						nullptr
						});

					if (!resourceManagers.mShaderBufferManager.isValid(brickCounterHandle)) {
						return;
					}

					// Clear the buffers...
					resourceManagers.mShaderBufferManager.transact(brickCounterHandle, [numBricks](ShaderBuffer& buffer) {
						uint32_t zero = 0u;;
						buffer.update(sizeof(uint32_t), reinterpret_cast<uint8_t*>(&zero));
						});
				}
				// Second, walk through each brick and assign it an ID
				renderPasses.computePass([brickIDsHandle, brickCounterHandle, bricksPerAxis](const ResourceManagers& resourceManagers, const ECS& ecs) {
					TRACY_GPUN("Count Bricks");
					auto brickIDsShaderHandle = resourceManagers.mShaderManager.asyncLoad("BrickCountShader", ShaderBuilder{}
						.setStage(types::shader::Stage::Compute, "vct/brickIDs.compute")
					);
					if (!resourceManagers.mShaderManager.isValid(brickIDsShaderHandle)) {
						return;
					}

					auto volumeView = ecs.getSingleView<VolumeComponent, SpatialComponent>();
					if (!volumeView) {
						return;
					}

					const auto& [_, volume, volumeSpatial] = *volumeView;

					MakeDefine(VOXELS_PER_BRICK_4);
					MakeDefine(VOXELS_PER_BRICK_8);
					ShaderDefines shaderDefines;
					if (volume.mVoxelsPerBrick == 4) {
						shaderDefines.set(VOXELS_PER_BRICK_4);
					}
					else if (volume.mVoxelsPerBrick == 8) {
						shaderDefines.set(VOXELS_PER_BRICK_8);
					}
					else {
						NEO_LOG_E("Unsupported voxels per brick count %d", volume.mVoxelsPerBrick);
						return;
					}

					auto resolvedShader = resourceManagers.mShaderManager.resolveDefines(brickIDsShaderHandle, shaderDefines);

					resolvedShader.bindShaderBuffer("BrickIDs", resourceManagers.mShaderBufferManager.resolve(brickIDsHandle), types::shader::Access::Read);
					resolvedShader.bindShaderBuffer("BrickCounter", resourceManagers.mShaderBufferManager.resolve(brickCounterHandle), types::shader::Access::ReadWrite);
					resolvedShader.bindUniform("volumeDimension", volume.mDimension);
					resolvedShader.bindUniform("brickDimension", volume.mVoxelsPerBrick);
					resolvedShader.bindUniform("bricksPerAxis", bricksPerAxis);

					int groupSize = (bricksPerAxis + volume.mVoxelsPerBrick - 1) / volume.mVoxelsPerBrick; // ceil(bricksPerAxis / boxelsPerBrick)
					resolvedShader.dispatch(glm::uvec3(groupSize));
					ShaderBarrier barrier(types::shader::Barrier::StorageBuffer);
					}, "BrickIDs");
			}

			return std::make_pair(brickIDsHandle, brickCounterHandle);
		}
	}

	std::pair<ShaderBufferHandle, ShaderBufferHandle> voxelize(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs) {
		TRACY_ZONE();
		auto [headerPointersHandle, voxelNodesHandle] = _generateVoxelNodes(renderPasses, resourceManagers, ecs);
		auto [brickIDsHandle, brickCounterHandle] = _generateBrickIDs(renderPasses, resourceManagers, ecs, headerPointersHandle, voxelNodesHandle);
		return std::make_pair(headerPointersHandle, voxelNodesHandle);
	}

	void debugVoxelNodes(FramebufferHandle outputHandle, glm::uvec2 viewport, RenderPasses& renderPasses, const ECS& ecs, ShaderBufferHandle headerBuffer, ShaderBufferHandle voxelNodesBuffer, ECS::Entity cameraEntity) {
		TRACY_ZONE();

		RenderState blendState;
		blendState.mBlendState = BlendState{
			BlendEquation::Add,
			BlendFuncSrc::Alpha,
			BlendFuncDst::OneMinusSrcAlpha
		};
		{
			auto volumeView = ecs.getSingleView<VolumeComponent, SpatialComponent, BoundingBoxComponent>();
			if (!volumeView) {
				return;
			}
			const auto& [_, volume, volumeSpatial, volumeBB] = *volumeView;
			glm::vec3 volumeWorldMin = glm::vec3(volumeSpatial.getModelMatrix() * glm::vec4(volumeBB.mMin, 1.0));
			glm::vec3 volumeWorldMax = glm::vec3(volumeSpatial.getModelMatrix() * glm::vec4(volumeBB.mMax, 1.0));

			glm::vec3 camPos = ecs.cGetComponent<SpatialComponent>(cameraEntity)->getPosition();
			bool cameraInside =
				camPos.x >= glm::min(volumeWorldMin.x, volumeWorldMax.x) && camPos.x <= glm::max(volumeWorldMin.x, volumeWorldMax.x) &&
				camPos.y >= glm::min(volumeWorldMin.y, volumeWorldMax.y) && camPos.y <= glm::max(volumeWorldMin.y, volumeWorldMax.y) &&
				camPos.z >= glm::min(volumeWorldMin.z, volumeWorldMax.z) && camPos.z <= glm::max(volumeWorldMin.z, volumeWorldMax.z);
			if (cameraInside) {
				blendState.mCullFace = CullFace::Front;
			}
		}
		blendState.mWireframeable = false;

		renderPasses.renderPass(outputHandle, viewport, blendState, [cameraEntity, voxelNodesBuffer, headerBuffer](const ResourceManagers& resourceManagers, const ECS& ecs) {
			TRACY_GPUN("Debug VoxelNodes");
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
			voxelNodeShader.bindUniform("cameraDir", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getLookDir());
			voxelNodeShader.bindShaderBuffer("VoxelNodes", resourceManagers.mShaderBufferManager.resolve(voxelNodesBuffer), types::shader::Access::Read);
			voxelNodeShader.bindShaderBuffer("HeaderPointers", resourceManagers.mShaderBufferManager.resolve(headerBuffer), types::shader::Access::Read);

			auto& mesh = resourceManagers.mMeshManager.resolve(MeshHandle("cube"));
			voxelNodeShader.bindUniform("M", volumeSpatial.getModelMatrix());
			mesh.draw();
		});
	}
}