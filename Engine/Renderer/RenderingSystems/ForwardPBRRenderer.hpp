#pragma once

#include "ECS/ECS.hpp"
#include "Util/Profiler.hpp"

#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"
#include "ECS/Component/RenderingComponent/TransparentComponent.hpp"
#include "ECS/Component/RenderingComponent/ForwardPBRRenderComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/OpaqueComponent.hpp"
#include "ECS/Component/RenderingComponent/IBLComponent.hpp"

#include "ECS/Component/CameraComponent/ShadowCameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"

#include "ECS/Component/LightComponent/MainLightComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/LightComponent/DirectionalLightComponent.hpp"
#include "ECS/Component/LightComponent/PointLightComponent.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"
#include "Renderer/FrameGraph/FrameGraph.hpp"

namespace neo {

	template<typename... CompTs, typename... Deps>
	inline void drawForwardPBR(
		FrameGraph& fg, 
		FramebufferHandle outTarget,
		Viewport vp,
		const ResourceManagers& resourceManagers, 
		const ECS& ecs, 
		const ECS::Entity cameraEntity, 
		Deps... deps
	) {
		TRACY_ZONE();

		auto pbrShaderHandle = resourceManagers.mShaderManager.asyncLoad("ForwardPBR Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "model.vert"},
			{ types::shader::Stage::Fragment, "forwardpbr.frag" }
			});

		const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
		if constexpr ((std::is_same_v<TransparentComponent, CompTs> || ...)) {
			TRACY_ZONEN("Transparency sorting");
			ecs.sort<ForwardPBRRenderComponent, TransparentComponent>([&cameraSpatial, &ecs](const ECS::Entity entityLeft, const ECS::Entity entityRight) {
				auto leftSpatial = ecs.cGetComponent<SpatialComponent>(entityLeft);
				auto rightSpatial = ecs.cGetComponent<SpatialComponent>(entityRight);
				if (leftSpatial && rightSpatial) {
					return glm::distance(cameraSpatial->getPosition(), leftSpatial->getPosition()) > glm::distance(cameraSpatial->getPosition(), rightSpatial->getPosition());
				}
				return false;
			});
		}

		// Forward draws are only lit by the single MainLightComponent
		const auto& lightView = ecs.getSingleView<MainLightComponent, LightComponent, SpatialComponent>();
		if (!lightView) {
			return;
		}

		auto&& [lightEntity, _mainLight, light, lightSpatial] = *lightView;
		const bool shadowsEnabled = 
			ecs.has<CameraComponent>(lightEntity) 
			&& ecs.has<ShadowCameraComponent>(lightEntity) 
			&& resourceManagers.mTextureManager.isValid(ecs.cGetComponent<ShadowCameraComponent>(lightEntity)->mShadowMap);

		PassState passState;
		if constexpr ((std::is_same_v<TransparentComponent, CompTs> || ...)) {
			passState.mBlending = true;
			passState.mBlendEquation = BlendEquation::Add;
			passState.mBlendSrcRGB = passState.mBlendSrcAlpha = BlendFactor::Alpha;
			passState.mBlendDstRGB = passState.mBlendDstAlpha = BlendFactor::OneMinusAlpha;
		}
		fg.pass(outTarget, vp, vp, passState, pbrShaderHandle)
			.with([light, shadowsEnabled, lightEntity, lightSpatial, cameraEntity, cameraSpatial](Pass& pass, const ResourceManagers& resourceManagers, const ECS& ecs) {
			TRACY_ZONEN("Forward PBR");
				MakeDefine(ALPHA_TEST);
				if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
					pass.setDefine(ALPHA_TEST);
				}

				MakeDefine(TRANSPARENT);
				if constexpr ((std::is_same_v<TransparentComponent, CompTs> || ...)) {
					pass.setDefine(TRANSPARENT);
				}
				MakeDefine(ENABLE_SHADOWS);
				MakeDefine(DIRECTIONAL_LIGHT);
				MakeDefine(POINT_LIGHT);
				pass.bindUniform("lightRadiance", glm::vec4(light.mColor, light.mIntensity));
				if (shadowsEnabled) {
					pass.setDefine(ENABLE_SHADOWS);
					auto& shadowMap = resourceManagers.mTextureManager.resolve(ecs.cGetComponent<ShadowCameraComponent>(lightEntity)->mShadowMap);
					pass.bindTexture("shadowMap", ecs.cGetComponent<ShadowCameraComponent>(lightEntity)->mShadowMap);
					pass.bindUniform("shadowMapResolution", glm::vec2(shadowMap.mWidth, shadowMap.mHeight));
				}
				if (ecs.has<DirectionalLightComponent>(lightEntity)) {
					pass.setDefine(DIRECTIONAL_LIGHT);
					pass.bindUniform("lightDir", -lightSpatial.getLookDir());
					if (shadowsEnabled) {
						const auto& shadowCamera = *ecs.cGetComponent<CameraComponent>(lightEntity);
						static glm::mat4 biasMatrix(
							0.5f, 0.0f, 0.0f, 0.0f,
							0.0f, 0.5f, 0.0f, 0.0f,
							0.0f, 0.0f, 0.5f, 0.0f,
							0.5f, 0.5f, 0.5f, 1.0f);
						pass.bindUniform("L", biasMatrix * shadowCamera.getProj() * lightSpatial.getView());
					}
				}
				else if (ecs.has<PointLightComponent>(lightEntity)) {
					pass.setDefine(POINT_LIGHT);
					pass.bindUniform("lightPos", lightSpatial.getPosition());
					pass.bindUniform("lightRadius", lightSpatial.getScale().x / 2.f);
					if (shadowsEnabled) {
						pass.bindUniform("shadowRange", static_cast<float>(lightSpatial.getScale().x) / 2.f);
					}
				}
				else {
					NEO_FAIL("Invalid light entity");
				}

				MakeDefine(IBL);
				if (const auto iblView = ecs.getSingleView<IBLComponent, SkyboxComponent>()) {
					const auto& ibl = std::get<1>(*iblView);
					if (ibl.mConvolved && ibl.mDFGGenerated && resourceManagers.mTextureManager.isValid(ibl.mConvolvedSkybox) && resourceManagers.mTextureManager.isValid(ibl.mDFGLut)) {
						pass.setDefine(IBL);
						const auto& iblTexture = resourceManagers.mTextureManager.resolve(ibl.mConvolvedSkybox);
						pass.bindTexture("dfgLUT", ibl.mDFGLut);
						pass.bindTexture("ibl", ibl.mConvolvedSkybox);
						pass.bindUniform("iblMips", iblTexture.mFormat.mMipCount - 1);
					}
				}

				pass.bindUniform("P", ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj());
				pass.bindUniform("V", cameraSpatial->getView());
				pass.bindUniform("camPos", cameraSpatial->getPosition());

				const auto& view = ecs.getView<const ForwardPBRRenderComponent, const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
				for (auto entity : view) {
					// VFC
					if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
						if (!culled->isInView(ecs, entity, cameraEntity)) {
							continue;
						}
					}
		
					ShaderDefinesFG drawDefines;
					UniformBuffer uniforms;

					const auto& material = view.get<const MaterialComponent>(entity);
					MakeDefine(ALBEDO_MAP);
					uniforms.bindUniform("albedo", material.mAlbedoColor);
					if (resourceManagers.mTextureManager.isValid(material.mAlbedoMap)) {
						drawDefines.set(ALBEDO_MAP);
						uniforms.bindTexture("albedoMap", material.mAlbedoMap);
					}
					MakeDefine(NORMAL_MAP);
					if (resourceManagers.mTextureManager.isValid(material.mNormalMap)) {
						drawDefines.set(NORMAL_MAP);
						uniforms.bindTexture("normalMap", material.mNormalMap);
					}
					MakeDefine(METAL_ROUGHNESS_MAP);
					uniforms.bindUniform("metalness", material.mMetallic);
					uniforms.bindUniform("roughness", material.mRoughness);
					if (resourceManagers.mTextureManager.isValid(material.mMetallicRoughnessMap)) {
						drawDefines.set(METAL_ROUGHNESS_MAP);
						uniforms.bindTexture("metalRoughnessMap", material.mMetallicRoughnessMap);
					}
					MakeDefine(OCCLUSION_MAP);
					if (resourceManagers.mTextureManager.isValid(material.mOcclusionMap)) {
						drawDefines.set(OCCLUSION_MAP);
						uniforms.bindTexture("occlusionMap", material.mOcclusionMap);
					}
					MakeDefine(EMISSIVE);
					uniforms.bindUniform("emissiveFactor", material.mEmissiveFactor);
					if (resourceManagers.mTextureManager.isValid(material.mEmissiveMap)) {
						drawDefines.set(EMISSIVE);
						uniforms.bindTexture("emissiveMap", material.mEmissiveMap);
					}
		
					const auto& mesh = resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle);
					MakeDefine(TANGENTS);
					if (mesh.hasVBO(types::mesh::VertexType::Tangent)) {
						drawDefines.set(TANGENTS);
					}
	
					const auto& drawSpatial = view.get<const SpatialComponent>(entity);
					uniforms.bindUniform("M", drawSpatial.getModelMatrix());
					uniforms.bindUniform("N", drawSpatial.getNormalMatrix());
		
					pass.drawCommand(view.get<const MeshComponent>(entity).mMeshHandle, uniforms, drawDefines);
				}

			})
			.dependsOn(resourceManagers, shadowsEnabled ? ecs.cGetComponent<ShadowCameraComponent>(lightEntity)->mShadowMap : NEO_INVALID_HANDLE)
			.setDebugName("ForwardPBR");

	}
}