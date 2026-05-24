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
					uint32_t albedo;
					uint32_t normal; // w is unused hmmm
					uint32_t emissive;
					int32_t header;
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
					buffer.clear(numNodes * sizeof(VoxelNode), 0);
					});
				resourceManagers.mShaderBufferManager.transact(headerPointersHandle, [numVoxels](ShaderBuffer& buffer) {
					buffer.clear(numVoxels * sizeof(int), -1);
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

						glm::vec3 volumeWorldMin = glm::vec3(volumeSpatial.getModelMatrix() * glm::vec4(volumeBB.mMin, 1.0));
						glm::vec3 volumeWorldMax = glm::vec3(volumeSpatial.getModelMatrix() * glm::vec4(volumeBB.mMax, 1.0));

						ShaderDefines defines;
						MakeDefine(ALBEDO_MAP);
						defines.set(ALBEDO_MAP);

						auto& albedoShader = resourceManagers.mShaderManager.resolveDefines(voxelNodeShaderHandle, {});
						auto& albedoMapShader = resourceManagers.mShaderManager.resolveDefines(voxelNodeShaderHandle, defines);
						auto outerBind = [&](auto& shader) {
							shader.bind();
							shader.bindUniform("volumeMin", glm::min(volumeWorldMin, volumeWorldMax));
							shader.bindUniform("volumeMax", glm::max(volumeWorldMin, volumeWorldMax));
							shader.bindUniform("volumeDimension", volume.mDimension);
							shader.bindUniform("numVoxels", numVoxels);
							shader.bindUniform("numNodes", numNodes);
							shader.bindUniform("P", volumeCamera.getProj());
							shader.bindUniform("V", volumeSpatial.getView());
						};
						outerBind(albedoShader);
						outerBind(albedoMapShader);
						auto bufferBarrierA  = albedoShader.bindShaderBuffer("VoxelNodes", resourceManagers.mShaderBufferManager.resolve(voxelNodesHandle), types::shader::Access::ReadWrite);
						auto bufferBarrierB  = albedoMapShader.bindShaderBuffer("VoxelNodes", resourceManagers.mShaderBufferManager.resolve(voxelNodesHandle), types::shader::Access::ReadWrite);
						auto headerBarrierA  = albedoShader.bindShaderBuffer("HeaderPointers", resourceManagers.mShaderBufferManager.resolve(headerPointersHandle), types::shader::Access::ReadWrite);
						auto headerBarrierB  = albedoMapShader.bindShaderBuffer("HeaderPointers", resourceManagers.mShaderBufferManager.resolve(headerPointersHandle), types::shader::Access::ReadWrite);
						auto counterBarrierA = albedoShader.bindShaderBuffer("NodeCounter", resourceManagers.mShaderBufferManager.resolve(atomicCounterHandle), types::shader::Access::ReadWrite);
						auto counterBarrierB = albedoMapShader.bindShaderBuffer("NodeCounter", resourceManagers.mShaderBufferManager.resolve(atomicCounterHandle), types::shader::Access::ReadWrite);

						const auto& meshView = ecs.getView<
							const VoxelizeComponent,
							const MeshComponent,
							const MaterialComponent,
							const SpatialComponent>();
						for (auto entity : meshView) {
							defines.reset();

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
							auto innerBind = [&](auto& shader) {
								shader.bind();
								shader.bindUniform("M", meshSpatial->getModelMatrix());
								shader.bindUniform("N", meshSpatial->getNormalMatrix());
								shader.bindUniform("albedo", material->mAlbedoColor);
								shader.bindUniform("emissive", material->mEmissiveFactor);
								if (resourceManagers.mTextureManager.isValid(material->mAlbedoMap)) {
									shader.bindTexture("albedoMap", resourceManagers.mTextureManager.resolve(material->mAlbedoMap));
								}
							};

							innerBind(resourceManagers.mTextureManager.isValid(material->mAlbedoMap) ? albedoMapShader : albedoShader);
							resourceManagers.mMeshManager.resolve(ecs.cGetComponent<MeshComponent>(entity)->mMeshHandle).draw();
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
			auto nodesBarrier = voxelNodeShader.bindShaderBuffer("VoxelNodes", resourceManagers.mShaderBufferManager.resolve(voxelNodesBuffer), types::shader::Access::Read);
			auto headerBarrier = voxelNodeShader.bindShaderBuffer("HeaderPointers", resourceManagers.mShaderBufferManager.resolve(headerBuffer), types::shader::Access::Read);

			auto& mesh = resourceManagers.mMeshManager.resolve(MeshHandle("cube"));
			voxelNodeShader.bindUniform("M", volumeSpatial.getModelMatrix());
			mesh.draw();
		});
	}
}