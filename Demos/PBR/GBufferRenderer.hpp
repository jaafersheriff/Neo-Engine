#pragma once

#include "ECS/ECS.hpp"
#include "Util/Profiler.hpp"

#include "PBRDeferredComponent.hpp"

#include "ECS/Component/RenderingComponent/PhongRenderComponent.hpp"
#include "ECS/Component/RenderingComponent/OpaqueComponent.hpp"
#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/ShadowCameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"

#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/LightComponent/DirectionalLightComponent.hpp"
#include "ECS/Component/LightComponent/PointLightComponent.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace PBR {

	FramebufferHandle createGbuffer(const ResourceManagers& resourceManagers, glm::uvec2 dimension) {
		return resourceManagers.mFramebufferManager.asyncLoad("Gbuffer",
			FramebufferBuilder{}
			.setSize(dimension)
			// AlbedoAO
			.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGBA16_F })
			// Normal
			.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGB16_F })
			// World roughness - World could go away if you do everything in view space...
			.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGBA16_F })
			// Emissive Metalness
			.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGBA16_F })
			// Depth
			.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::D16 }),
			resourceManagers.mTextureManager
		);

	}

	template<typename... CompTs>
	void drawGBuffer(
		const ResourceManagers& resourceManagers, 
		const ECS& ecs, 
		const ECS::Entity cameraEntity, 
		const FramebufferHandle gbufferHandle,
		const ShaderDefines& inDefines = {}
	) {
		TRACY_GPU();

		if (!resourceManagers.mFramebufferManager.isValid(gbufferHandle)) {
			return;
		}
		auto& gbuffer = resourceManagers.mFramebufferManager.resolve(gbufferHandle);
		gbuffer.bind();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("GBuffer Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "model.vert"},
			{ types::shader::Stage::Fragment, "pbr/gbuffer.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
			return;
		}

		ShaderDefines passDefines(inDefines);
		MakeDefine(ALPHA_TEST);
		if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
			passDefines.set(ALPHA_TEST);
			// Transparency sorting..for later
		//	 glEnable(GL_BLEND);
		//	 ecs.sort<AlphaTestComponent>([&cameraSpatial, &ecs](ECS::Entity entityLeft, ECS::Entity entityRight) {
		//		 auto leftSpatial = ecs.cGetComponent<SpatialComponent>(entityLeft);
		//		 auto rightSpatial = ecs.cGetComponent<SpatialComponent>(entityRight);
		//		 if (leftSpatial && rightSpatial) {
		//			 return glm::distance(cameraSpatial->getPosition(), leftSpatial->getPosition()) < glm::distance(cameraSpatial->getPosition(), rightSpatial->getPosition());
		//		 }
		//		 return false;
		//		 });
		}

		ShaderDefines drawDefines(passDefines);
		const auto& view = ecs.getView<const PBRDeferredComponent, const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
		for (auto entity : view) {
			// VFC
			if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
				if (!culled->isInView(ecs, entity, cameraEntity)) {
					continue;
				}
			}

			drawDefines.reset();
			const auto& material = view.get<const MaterialComponent>(entity);
			MakeDefine(ALBEDO_MAP);
			MakeDefine(NORMAL_MAP);
			MakeDefine(METAL_ROUGHNESS_MAP);
			MakeDefine(OCCLUSION_MAP);
			MakeDefine(EMISSIVE);
			if (resourceManagers.mTextureManager.isValid(material.mAlbedoMap)) {
				drawDefines.set(ALBEDO_MAP);
			}
			if (resourceManagers.mTextureManager.isValid(material.mNormalMap)) {
				drawDefines.set(NORMAL_MAP);
			}
			if (resourceManagers.mTextureManager.isValid(material.mMetallicRoughnessMap)) {
				drawDefines.set(METAL_ROUGHNESS_MAP);
			}
			if (resourceManagers.mTextureManager.isValid(material.mOcclusionMap)) {
				drawDefines.set(OCCLUSION_MAP);
			}
			if (resourceManagers.mTextureManager.isValid(material.mEmissiveMap)) {
				drawDefines.set(EMISSIVE);
			}

			const auto& mesh = resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle);
			MakeDefine(TANGENTS);
			if (mesh.hasVBO(types::mesh::VertexType::Tangent)) {
				drawDefines.set(TANGENTS);
			}

			auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, drawDefines);
			resolvedShader.bind();

			resolvedShader.bindUniform("albedo", material.mAlbedoColor);
			if (resourceManagers.mTextureManager.isValid(material.mAlbedoMap)) {
				resolvedShader.bindTexture("albedoMap", resourceManagers.mTextureManager.resolve(material.mAlbedoMap));
			}

			if (resourceManagers.mTextureManager.isValid(material.mNormalMap)) {
				resolvedShader.bindTexture("normalMap", resourceManagers.mTextureManager.resolve(material.mNormalMap));
			}

			resolvedShader.bindUniform("metalness", material.mMetallic);
			resolvedShader.bindUniform("roughness", material.mRoughness);
			if (resourceManagers.mTextureManager.isValid(material.mMetallicRoughnessMap)) {
				resolvedShader.bindTexture("metalRoughnessMap", resourceManagers.mTextureManager.resolve(material.mMetallicRoughnessMap));
			}

			if (resourceManagers.mTextureManager.isValid(material.mOcclusionMap)) {
				resolvedShader.bindTexture("occlusionMap", resourceManagers.mTextureManager.resolve(material.mOcclusionMap));
			}

			resolvedShader.bindUniform("emissiveFactor", material.mEmissiveFactor);
			if (resourceManagers.mTextureManager.isValid(material.mEmissiveMap)) {
				resolvedShader.bindTexture("emissiveMap", resourceManagers.mTextureManager.resolve(material.mEmissiveMap));
			}

			// UBO candidates
			{
				resolvedShader.bindUniform("P", ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj());
				resolvedShader.bindUniform("V", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getView());
			}

			const auto& drawSpatial = view.get<const SpatialComponent>(entity);
			resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
			resolvedShader.bindUniform("N", drawSpatial.getNormalMatrix());

			resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle).draw();
		}
	}
}
