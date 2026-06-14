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

#include "Util/ServiceLocator.hpp"

namespace VCT {

	using namespace neo;

	namespace {
		std::pair<TextureHandle, TextureHandle> _generateBricks(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs) {
			TRACY_ZONE();
			auto volumeView = ecs.getSingleView<VolumeComponent, SpatialComponent>();
			if (!volumeView) {
				return {};
			}

			const auto& [_, volume, volumeSpatial] = *volumeView;
			const uint16_t logicalBricksPerAxis = volume.getLogicalBricksPerAxis();
			const uint16_t physicalBricksPerAxis = volume.getPhysicalBricksPerAxis();
			const uint16_t brickSize = volume.mVoxelsPerBrick + 2; // padding

			auto brickPointersHandle = resourceManagers.mTextureManager.asyncLoad("brickPointers", TextureBuilder{}
				.setFormat(TextureFormat{ 
					types::texture::Target::Texture3D, 
					types::InternalFormats::R32_I, 
					TextureFilter{types::texture::Filters::Nearest, types::texture::Filters::Nearest, types::texture::Filters::Nearest},
					TextureWrap{types::texture::Wraps::Clamp, types::texture::Wraps::Clamp, types::texture::Wraps::Clamp},
					types::ByteFormats::Int
				})
				.setDimension(glm::u16vec3(logicalBricksPerAxis))
			);
			auto bricksTextureHandle = resourceManagers.mTextureManager.asyncLoad("BricksTexture", TextureBuilder{}
				.setFormat(TextureFormat{ 
					types::texture::Target::Texture3D, 
					types::InternalFormats::RGBA32_UI, // packed u32
					TextureFilter{types::texture::Filters::Linear, types::texture::Filters::Linear, types::texture::Filters::Linear},
					TextureWrap{types::texture::Wraps::Clamp, types::texture::Wraps::Clamp, types::texture::Wraps::Clamp},
					types::ByteFormats::UnsignedByte
					// TODO - mips go here
					})
				.setDimension(glm::u16vec3(physicalBricksPerAxis * brickSize))
			);

			auto atomicCounterHandle = resourceManagers.mShaderBufferManager.asyncLoad("BrickIDCounter", ShaderBufferLoadDetails{
				sizeof(uint32_t),
				nullptr
				});

			{
				if (!resourceManagers.mTextureManager.isValid(brickPointersHandle)
					|| !resourceManagers.mTextureManager.isValid(bricksTextureHandle)
					|| !resourceManagers.mShaderBufferManager.isValid(atomicCounterHandle)
					) {
					return {};
				}

				// Resolution changed, destroy everything and try again next frame
				if (resourceManagers.mTextureManager.resolve(brickPointersHandle).mWidth != logicalBricksPerAxis) {
					resourceManagers.mTextureManager.discard(brickPointersHandle);
					resourceManagers.mTextureManager.discard(bricksTextureHandle);
					resourceManagers.mShaderBufferManager.discard(atomicCounterHandle);
					return {};
				}

				// Clear the buffers...
				resourceManagers.mTextureManager.transact(brickPointersHandle, [](Texture& texture) {
					int32_t clearValue = -1;
					texture.clear(reinterpret_cast<uint8_t*>(&clearValue));
					});
				resourceManagers.mTextureManager.transact(bricksTextureHandle, [](Texture& texture) {
					uint32_t clearValue[4] = { 0,0,0,0 };
					for (uint16_t i = 0; i < texture.mFormat.mMipCount; i++) {
						texture.clear(
							i,
							glm::uvec3(0),
							glm::uvec3(texture.mWidth, texture.mHeight, texture.mDepth),
							reinterpret_cast<uint8_t*>(&clearValue)
						);
					}
					});
				resourceManagers.mShaderBufferManager.transact(atomicCounterHandle, [](ShaderBuffer& buffer) {
					uint32_t zero = 0;
					buffer.update(sizeof(uint32_t), reinterpret_cast<const uint8_t*>(&zero));
					});
			}

			// This is just a placeholder in order to draw things.. 
			// colorwrite can also be disabled, but I like to see it as a debug vis 
			auto brickTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
				"Bricks Target",
				FramebufferBuilder{}
				.setSize(glm::uvec2(physicalBricksPerAxis, physicalBricksPerAxis))
				.attach(TextureFormat{ types::texture::Target::Texture2D, types::InternalFormats::RGB8_UNORM }),
				resourceManagers.mTextureManager
			);

			renderPasses.clear(brickTargetHandle, types::framebuffer::AttachmentBit::Color, glm::vec4(0.f)); // Probably unnecessary

			RenderState renderState;
			renderState.mDepthState = std::nullopt;
			renderState.mCullFace = std::nullopt;
			renderPasses.renderPass(brickTargetHandle, glm::uvec2(physicalBricksPerAxis), renderState,
				[atomicCounterHandle, brickPointersHandle, bricksTextureHandle](const ResourceManagers& resourceManagers, const ECS& ecs) {
					TRACY_GPUN("Generate Bricks");
					const auto& [_, volume, volumeSpatial, volumeCamera, volumeBB] = *ecs.getSingleView<VolumeComponent, SpatialComponent, CameraComponent, BoundingBoxComponent>();

					auto brickGenShaderHandle = resourceManagers.mShaderManager.asyncLoad("BrickGenShader", ShaderBuilder{}
						.setStage(types::shader::Stage::Vertex, "vct/brickGen.vert")
						.setStage(types::shader::Stage::Geometry, "vct/brickGen.geom")
						.setStage(types::shader::Stage::Fragment, "vct/brickGen.frag")
					);
					if (!resourceManagers.mShaderManager.isValid(brickGenShaderHandle)) {
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

						auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(brickGenShaderHandle, resourceManagers.mTextureManager.isValid(material->mAlbedoMap) ? albedoDefines : ShaderDefines{});

						resolvedShader.bindShaderBuffer("BrickCounter", resourceManagers.mShaderBufferManager.resolve(atomicCounterHandle), types::shader::Access::ReadWrite);
						resolvedShader.bindImageTexture("BrickPointers", resourceManagers.mTextureManager.resolve(brickPointersHandle), types::shader::Access::ReadWrite);
						resolvedShader.bindImageTexture("BrickTexture", resourceManagers.mTextureManager.resolve(bricksTextureHandle), types::shader::Access::ReadWrite);

						resolvedShader.bindUniform("volumeMin", glm::min(volumeWorldMin, volumeWorldMax));
						resolvedShader.bindUniform("volumeMax", glm::max(volumeWorldMin, volumeWorldMax));
						resolvedShader.bindUniform("volumeDimension", volume.mDimension);
						resolvedShader.bindUniform("brickSize", volume.mVoxelsPerBrick);
						resolvedShader.bindUniform("maxBricks", volume.mMaxBricks);
						resolvedShader.bindUniform("brickResolution", glm::ivec3(volume.getLogicalBricksPerAxis()));
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

					ShaderBarrier imageBarrier(types::shader::Barrier::ImageAccess);
					ShaderBarrier shaderBarrier(types::shader::Barrier::StorageBuffer);
				}, "Generate Bricks");

			return std::make_pair(brickPointersHandle, bricksTextureHandle);
		}
	}

	struct VoxelizationResult {
		TextureHandle mBrickPointers;
		TextureHandle mBricksTexture;
	};
	VoxelizationResult voxelize(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs) {
		TRACY_ZONE();

		auto volumeView = ecs.getSingleView<VolumeComponent, SpatialComponent>();
		if (!volumeView) {
			return {};
		}
		
		auto [pointersHandle, textureHandle] = _generateBricks(renderPasses, resourceManagers, ecs);
		return VoxelizationResult{ pointersHandle, textureHandle };
	}

	void debugVoxelNodes(FramebufferHandle outputHandle, glm::uvec2 viewport, RenderPasses& renderPasses, const ECS& ecs, VoxelizationResult buffers, bool debugBricks, ECS::Entity cameraEntity) {
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

		renderPasses.renderPass(outputHandle, viewport, blendState, [cameraEntity, buffers, debugBricks](const ResourceManagers& resourceManagers, const ECS& ecs) {
			TRACY_GPUN("Debug VoxelNodes");
			if (!resourceManagers.mTextureManager.isValid(buffers.mBrickPointers)
				|| !resourceManagers.mTextureManager.isValid(buffers.mBricksTexture))  {
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

			MakeDefine(DEBUG_BRICKS);
			ShaderDefines defines;
			if (debugBricks) {
				defines.set(DEBUG_BRICKS);
			}

			auto voxelNodeShader = resourceManagers.mShaderManager.resolveDefines(voxelNodeDebugShaderHandle, defines);
			voxelNodeShader.bind();

			glm::vec3 volumeWorldMin = glm::vec3(volumeSpatial.getModelMatrix() * glm::vec4(volumeBB.mMin, 1.0));
			glm::vec3 volumeWorldMax = glm::vec3(volumeSpatial.getModelMatrix() * glm::vec4(volumeBB.mMax, 1.0));

			voxelNodeShader.bindUniform("volumeMin", glm::min(volumeWorldMin, volumeWorldMax));
			voxelNodeShader.bindUniform("volumeMax", glm::max(volumeWorldMin, volumeWorldMax));
			voxelNodeShader.bindUniform("volumeDimension", volume.mDimension);
			voxelNodeShader.bindUniform("bricksPerAxis", volume.getLogicalBricksPerAxis());
			voxelNodeShader.bindUniform("voxelsPerBrick", volume.mVoxelsPerBrick);
			voxelNodeShader.bindUniform("P", ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj());
			voxelNodeShader.bindUniform("V", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getView());
			voxelNodeShader.bindUniform("cameraPos", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getPosition());
			voxelNodeShader.bindUniform("cameraDir", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getLookDir());

			voxelNodeShader.bindImageTexture("BrickPointers", resourceManagers.mTextureManager.resolve(buffers.mBrickPointers), types::shader::Access::Read);
			voxelNodeShader.bindImageTexture("BrickTexture", resourceManagers.mTextureManager.resolve(buffers.mBricksTexture), types::shader::Access::Read);

			auto& mesh = resourceManagers.mMeshManager.resolve(MeshHandle("cube"));
			voxelNodeShader.bindUniform("M", volumeSpatial.getModelMatrix());
			mesh.draw();
		});
	}
}