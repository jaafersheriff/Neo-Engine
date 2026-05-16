#pragma once

#include "VolumeComponent.hpp"
#include "VoxelizeComponent.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderingComponent/OpaqueComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"

#include "Renderer/RenderingSystems/RenderPass.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

namespace VCT {

	using namespace neo;

	void voxelize(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs) {
		TRACY_ZONE();
		auto volumeView = ecs.getSingleView<VolumeComponent, SpatialComponent>();
		if (!volumeView) {
			return;
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

#define SCENE_COMPLEXITY 128u
			int bufferSize = volume.mDimension * volume.mDimension * volume.mDimension * SCENE_COMPLEXITY;
			voxelFragmentsHandle = resourceManagers.mShaderBufferManager.asyncLoad("VolumeFragments", ShaderBufferLoadDetails{
				static_cast<uint32_t>(bufferSize * sizeof(VoxelFragment)),
				nullptr
				});

			if (!resourceManagers.mShaderBufferManager.isValid(voxelFragmentsHandle)) {
				return;
			}

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
				[bufferSize, voxelFragmentsHandle](const ResourceManagers& resourceManagers, const ECS& ecs) {
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
					auto barrier = voxelFragmentShader.bindShaderBuffer("VoxelFragments", resourceManagers.mShaderBufferManager.resolve(voxelFragmentsHandle), types::shader::Access::ReadWrite);

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
	}
}