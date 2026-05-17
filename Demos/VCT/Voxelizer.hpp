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

	ShaderBufferHandle voxelize(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs) {
		TRACY_ZONE();
		auto volumeView = ecs.getSingleView<VolumeComponent, SpatialComponent>();
		if (!volumeView) {
			return ShaderBufferHandle{};
		}

		const auto& [_, volume, volumeSpatial] = *volumeView;

		// Raster path to create voxel fragments
		ShaderBufferHandle voxelFragmentsHandle;
		{
			TRACY_ZONEN("VoxelFragments");
			struct VoxelFragment {
				glm::vec3 worldPosition;
				glm::vec4 albedo; // TODO : Packed RGBA
				glm::vec3 normal; // TODO : Packed Normal
			};

			int bufferSize = volume.mDimension * volume.mDimension * volume.mDimension;
			voxelFragmentsHandle = resourceManagers.mShaderBufferManager.asyncLoad("VolumeFragments", ShaderBufferLoadDetails{
				static_cast<uint32_t>(bufferSize * sizeof(VoxelFragment)),
				nullptr
				});
			std::vector<int> voxelLockData(bufferSize, -1);
			ShaderBufferHandle voxelLocksHandle = resourceManagers.mShaderBufferManager.asyncLoad("VoxelLocks", ShaderBufferLoadDetails{
				static_cast<uint32_t>(voxelLockData.size() * sizeof(int)),
				reinterpret_cast<const uint8_t*>(voxelLockData.data())
				});

			ShaderBufferHandle atomicCounterHandle = resourceManagers.mShaderBufferManager.asyncLoad("VolumeFragmentsAtomicCounter", ShaderBufferLoadDetails{
				sizeof(uint32_t),
				nullptr
				});

			if (!resourceManagers.mShaderBufferManager.isValid(voxelFragmentsHandle) 
				|| !resourceManagers.mShaderBufferManager.isValid(voxelLocksHandle)
				|| !resourceManagers.mShaderBufferManager.isValid(atomicCounterHandle)
				) {
				return ShaderBufferHandle{};
			}

			// Resolution changed, destroy everything and try again next frame
			if (resourceManagers.mShaderBufferManager.resolve(voxelFragmentsHandle).mByteSize != bufferSize * sizeof(VoxelFragment)) {
				resourceManagers.mShaderBufferManager.discard(voxelFragmentsHandle);
				resourceManagers.mShaderBufferManager.discard(voxelLocksHandle);
				resourceManagers.mShaderBufferManager.discard(atomicCounterHandle);
				return ShaderBufferHandle{};
			}

			// Clear the buffers...
			resourceManagers.mShaderBufferManager.transact(voxelFragmentsHandle, [bufferSize](ShaderBuffer& buffer) {
				std::vector<uint8_t> data(bufferSize * sizeof(VoxelFragment), 0);
				buffer.update(bufferSize * sizeof(VoxelFragment), data.data());
			});
			resourceManagers.mShaderBufferManager.transact(voxelLocksHandle, [bufferSize](ShaderBuffer& buffer) {
				std::vector<int> voxelLockData(bufferSize, -1);
				buffer.update(bufferSize * sizeof(int), reinterpret_cast<const uint8_t*>(voxelLockData.data()));
			});
			resourceManagers.mShaderBufferManager.transact(atomicCounterHandle, [](ShaderBuffer& buffer) {
				uint32_t zero = 0;
				buffer.update(sizeof(uint32_t), reinterpret_cast<const uint8_t*>(&zero));
			});

			auto voxelFragmentTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
				"Voxel Fragments",
				FramebufferBuilder{}
				.setSize(glm::uvec2(volume.mDimension, volume.mDimension))
				.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGB8_UNORM }),
				resourceManagers.mTextureManager
			);

			renderPasses.clear(voxelFragmentTargetHandle, types::framebuffer::AttachmentBit::Color, glm::vec4(0.f)); // Probably unnecessary

			RenderState renderState;
			renderState.mDepthState = std::nullopt;
			renderPasses.renderPass(voxelFragmentTargetHandle, glm::uvec2(volume.mDimension), RenderState{},
				[bufferSize, voxelFragmentsHandle, atomicCounterHandle, voxelLocksHandle](const ResourceManagers& resourceManagers, const ECS& ecs) {
					TRACY_GPUN("Voxel Fragments");
					const auto& [_, volume, volumeSpatial, volumeCamera] = *ecs.getSingleView<VolumeComponent, SpatialComponent, CameraComponent>();

					auto voxelFragmentShaderHandle = resourceManagers.mShaderManager.asyncLoad("VoxelFragmentsShader", ShaderBuilder{}
						.setStage(types::shader::Stage::Vertex, "vct/voxelfragments.vert")
						.setStage(types::shader::Stage::Geometry, "vct/voxelfragments.geom")
						.setStage(types::shader::Stage::Fragment, "vct/voxelfragments.frag")
					);
					if (!resourceManagers.mShaderManager.isValid(voxelFragmentShaderHandle)) {
						return;
					}

					auto voxelFragmentShader = resourceManagers.mShaderManager.resolveDefines(voxelFragmentShaderHandle, {});
					voxelFragmentShader.bind();

					voxelFragmentShader.bindUniform("volumeDimension", volume.mDimension);
					voxelFragmentShader.bindUniform("outputBufferSize", bufferSize);
					voxelFragmentShader.bindUniform("P", volumeCamera.getProj());
					voxelFragmentShader.bindUniform("V", volumeSpatial.getView());
					auto bufferBarrier = voxelFragmentShader.bindShaderBuffer("VoxelFragments", resourceManagers.mShaderBufferManager.resolve(voxelFragmentsHandle), types::shader::Access::ReadWrite);
					auto locksBarrier = voxelFragmentShader.bindShaderBuffer("VoxelLocks", resourceManagers.mShaderBufferManager.resolve(voxelLocksHandle), types::shader::Access::ReadWrite);
					auto counterBarrier = voxelFragmentShader.bindShaderBuffer("FragmentCounter", resourceManagers.mShaderBufferManager.resolve(atomicCounterHandle), types::shader::Access::ReadWrite);

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
							voxelFragmentShader.bindUniform("M", meshSpatial->getModelMatrix());
							voxelFragmentShader.bindUniform("N", meshSpatial->getNormalMatrix());
							voxelFragmentShader.bindUniform("albedo", glm::vec3(material->mAlbedoColor) + material->mEmissiveFactor);

							mesh.draw();
						}
					}
				}, "VoxelFragments");
		}

		return voxelFragmentsHandle;
	}

	void debugVoxelFragments(const ResourceManagers& resourceManagers, const ECS& ecs, ShaderBufferHandle voxelFragmentsBuffer, ECS::Entity cameraEntity) {
		TRACY_ZONE();
		if (!resourceManagers.mShaderBufferManager.isValid(voxelFragmentsBuffer)) {
			return;
		}

		auto volumeView = ecs.getSingleView<VolumeComponent, SpatialComponent, BoundingBoxComponent>();
		if (!volumeView) {
			return;
		}
		const auto& [_, volume, volumeSpatial, volumeBB] = *volumeView;

		auto voxelFragmentDebugShaderHandle = resourceManagers.mShaderManager.asyncLoad("VoxelFragmentsDebugShader", ShaderBuilder{}
			.setStage(types::shader::Stage::Vertex, "model.vert")
			.setStage(types::shader::Stage::Fragment, "vct/voxelfragmentsdebug.frag")
		);
		if (!resourceManagers.mShaderManager.isValid(voxelFragmentDebugShaderHandle)) {
			return;
		}

		auto voxelFragmentShader = resourceManagers.mShaderManager.resolveDefines(voxelFragmentDebugShaderHandle, {});
		voxelFragmentShader.bind();

		glm::vec3 volumeWorldMin = glm::vec3(volumeSpatial.getModelMatrix() * glm::vec4(volumeBB.mMin, 1.0));
		glm::vec3 volumeWorldMax = glm::vec3(volumeSpatial.getModelMatrix() * glm::vec4(volumeBB.mMax, 1.0));

		voxelFragmentShader.bindUniform("volumeDimension", volume.mDimension);
		voxelFragmentShader.bindUniform("volumeMin", glm::min(volumeWorldMin, volumeWorldMax));
		voxelFragmentShader.bindUniform("volumeMax", glm::max(volumeWorldMin, volumeWorldMax));
		voxelFragmentShader.bindUniform("P", ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj());
		voxelFragmentShader.bindUniform("V", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getView());
		voxelFragmentShader.bindUniform("cameraPos", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getPosition());
		auto barrier = voxelFragmentShader.bindShaderBuffer("VoxelFragments", resourceManagers.mShaderBufferManager.resolve(voxelFragmentsBuffer), types::shader::Access::Read);

		auto& mesh = resourceManagers.mMeshManager.resolve(MeshHandle("cube"));
		voxelFragmentShader.bindUniform("M", volumeSpatial.getModelMatrix());
		mesh.draw();
	}
}