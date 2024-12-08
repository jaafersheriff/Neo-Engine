#pragma once

#include "ECS/ECS.hpp"
#include "Util/Profiler.hpp"

#include "ECS/Component/CameraComponent/CSMCameraComponent.hpp"

#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"
#include "ECS/Component/RenderingComponent/TransparentComponent.hpp"
#include "ECS/Component/RenderingComponent/ForwardPBRRenderComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/RenderingComponent/OpaqueComponent.hpp"
#include "ECS/Component/RenderingComponent/IBLComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/RenderingComponent/ShadowMapComponents.hpp"

#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Component/LightComponent/MainLightComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/LightComponent/DirectionalLightComponent.hpp"
#include "ECS/Component/LightComponent/PointLightComponent.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	template<typename... CompTs>
	void drawForwardPBR(const ResourceManagers& resourceManagers, const ECS& ecs, const ECS::Entity cameraEntity, std::optional<IBLComponent> ibl = std::nullopt) {
		TRACY_GPU();

		// Forward draws are only lit by the single MainLightComponent!
		const auto& lightView = ecs.getSingleView<MainLightComponent, LightComponent, SpatialComponent>();
		if (!lightView) {
			return;
		}
		auto&& [lightEntity, _mainLight, light, lightSpatial] = *lightView;

		auto pbrShaderHandle = resourceManagers.mShaderManager.asyncLoad("ForwardPBR Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "model.vert"},
			{ types::shader::Stage::Fragment, "forwardpbr.frag" }
			});
		if (!resourceManagers.mShaderManager.isValid(pbrShaderHandle)) {
			return;
		}

		ShaderDefines passDefines({});
		bool containsAlphaTest = false;
		MakeDefine(ALPHA_TEST);
		if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
			containsAlphaTest = true;
			passDefines.set(ALPHA_TEST);
		}

		bool containsTransparency = false;
		MakeDefine(TRANSPARENT);
		if constexpr ((std::is_same_v<TransparentComponent, CompTs> || ...)) {
			containsTransparency = true;
			passDefines.set(TRANSPARENT);

			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		bool directionalLight = ecs.has<DirectionalLightComponent>(lightEntity);
		MakeDefine(DIRECTIONAL_LIGHT);

		bool pointLight = ecs.has<PointLightComponent>(lightEntity);
		MakeDefine(POINT_LIGHT);

		bool shadowsEnabled = false;
		MakeDefine(ENABLE_SHADOWS);
		std::array<glm::mat4, 4> lightArrays;
		glm::vec4 csmDepths(0.f);

		if (directionalLight) {
			passDefines.set(DIRECTIONAL_LIGHT);
			shadowsEnabled = true
				&& ecs.has<CameraComponent>(lightEntity)
				&& ecs.has<CSMShadowMapComponent>(lightEntity)
				&& resourceManagers.mTextureManager.isValid(ecs.cGetComponent<CSMShadowMapComponent>(lightEntity)->mShadowMap)
				&& ecs.entityCount<CSMCamera0Component>()
				&& ecs.entityCount<CSMCamera1Component>()
				&& ecs.entityCount<CSMCamera2Component>()
				&& ecs.entityCount<CSMCamera3Component>();
			if (shadowsEnabled) {
				passDefines.set(ENABLE_SHADOWS);
				static glm::mat4 biasMatrix(
					0.5f, 0.0f, 0.0f, 0.0f,
					0.0f, 0.5f, 0.0f, 0.0f,
					0.0f, 0.0f, 0.5f, 0.0f,
					0.5f, 0.5f, 0.5f, 1.0f);
				auto csmCamera0Tuple = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera0Component>();
				auto csmCamera1Tuple = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera1Component>();
				auto csmCamera2Tuple = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera2Component>();
				auto csmCamera3Tuple = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera3Component>();
				NEO_ASSERT(csmCamera0Tuple && csmCamera1Tuple && csmCamera2Tuple && csmCamera3Tuple, "CSM Camera's dont exist");
				auto& [cameraEntity0, cameraSpatial0, csmCameraCamera0, csmCamera0] = *csmCamera0Tuple;
				auto& [cameraEntity1, cameraSpatial1, csmCameraCamera1, csmCamera1] = *csmCamera1Tuple;
				auto& [cameraEntity2, cameraSpatial2, csmCameraCamera2, csmCamera2] = *csmCamera2Tuple;
				auto& [cameraEntity3, cameraSpatial3, csmCameraCamera3, csmCamera3] = *csmCamera3Tuple;

				lightArrays[0] = biasMatrix * csmCameraCamera0.getProj() * cameraSpatial0.getView();
				lightArrays[1] = biasMatrix * csmCameraCamera1.getProj() * cameraSpatial1.getView();
				lightArrays[2] = biasMatrix * csmCameraCamera2.getProj() * cameraSpatial2.getView();
				lightArrays[3] = biasMatrix * csmCameraCamera3.getProj() * cameraSpatial3.getView();
				csmDepths.x = csmCamera0.mSliceDepth;
				csmDepths.y = csmCamera1.mSliceDepth;
				csmDepths.z = csmCamera2.mSliceDepth;
				csmDepths.w = csmCamera3.mSliceDepth;
			}
		}
		else if (pointLight) {
			passDefines.set(POINT_LIGHT);
			shadowsEnabled =
				ecs.has<PointLightShadowMapComponent>(lightEntity)
				&& resourceManagers.mTextureManager.isValid(ecs.cGetComponent<PointLightShadowMapComponent>(lightEntity)->mShadowMap);
			if (shadowsEnabled) {
				passDefines.set(ENABLE_SHADOWS);
			}
		}
		else {
			NEO_FAIL("Invalid light entity");
		}

		MakeDefine(IBL);
		if (ibl && resourceManagers.mTextureManager.isValid(ibl->mConvolvedSkybox) && resourceManagers.mTextureManager.isValid(ibl->mDFGLut)) {
			passDefines.set(IBL);
		}

		const glm::mat4 P = ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj();
		const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);

		ShaderDefines drawDefines(passDefines);
		if (containsTransparency) {
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
		const auto& view = ecs.getView<const ForwardPBRRenderComponent, const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
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

			auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(pbrShaderHandle, drawDefines);
			resolvedShader.bind();

			resolvedShader.bindUniform("albedo", material.mAlbedoColor);
			if (resourceManagers.mTextureManager.isValid(material.mAlbedoMap)) {
				resolvedShader.bindTexture("albedoMap", resourceManagers.mTextureManager.resolve(material.mAlbedoMap));
			}

			if (resourceManagers.mTextureManager.isValid(material.mNormalMap)) {
				resolvedShader.bindTexture("normalMap", resourceManagers.mTextureManager.resolve(material.mNormalMap));
				resolvedShader.bindUniform("normalMapScale", material.mNormalScale);
			}

			resolvedShader.bindUniform("metalness", material.mMetallic);
			resolvedShader.bindUniform("roughness", material.mRoughness);
			if (resourceManagers.mTextureManager.isValid(material.mMetallicRoughnessMap)) {
				resolvedShader.bindTexture("metalRoughnessMap", resourceManagers.mTextureManager.resolve(material.mMetallicRoughnessMap));
			}

			if (resourceManagers.mTextureManager.isValid(material.mOcclusionMap)) {
				resolvedShader.bindTexture("occlusionMap", resourceManagers.mTextureManager.resolve(material.mOcclusionMap));
				resolvedShader.bindUniform("occlusionStrength", material.mOcclusionStrength);
			}

			resolvedShader.bindUniform("emissiveFactor", material.mEmissiveFactor);
			if (resourceManagers.mTextureManager.isValid(material.mEmissiveMap)) {
				resolvedShader.bindTexture("emissiveMap", resourceManagers.mTextureManager.resolve(material.mEmissiveMap));
			}

			// UBO candidates
			{
				resolvedShader.bindUniform("P", P);
				resolvedShader.bindUniform("V", cameraSpatial->getView());
				resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());
				resolvedShader.bindUniform("lightRadiance", glm::vec4(light.mColor, light.mIntensity));
				if (directionalLight) {
					resolvedShader.bindUniform("lightDir", -lightSpatial.getLookDir());
				}
				if (pointLight) {
					resolvedShader.bindUniform("lightPos", lightSpatial.getPosition());
					resolvedShader.bindUniform("lightRadius", lightSpatial.getScale().x / 2.f);
				}
				if (shadowsEnabled) {
					TextureHandle shadowMapHandle;
					if (directionalLight) {
						shadowMapHandle = ecs.cGetComponent<CSMShadowMapComponent>(lightEntity)->mShadowMap;
						resolvedShader.bindUniform("L0", lightArrays[0]);
						resolvedShader.bindUniform("L1", lightArrays[1]);
						resolvedShader.bindUniform("L2", lightArrays[2]);
						resolvedShader.bindUniform("L3", lightArrays[3]);
						resolvedShader.bindUniform("csmDepths", csmDepths);
					}
					if (pointLight) {
						shadowMapHandle = ecs.cGetComponent<PointLightShadowMapComponent>(lightEntity)->mShadowMap;
						resolvedShader.bindUniform("shadowRange", static_cast<float>(lightSpatial.getScale().x) / 2.f);
					}
					const auto& shadowMap = resourceManagers.mTextureManager.resolve(shadowMapHandle);
					resolvedShader.bindTexture("shadowMap", shadowMap);
					resolvedShader.bindUniform("shadowMapResolution", glm::vec2(shadowMap.mWidth, shadowMap.mHeight));
				}
				if (ibl) {
					const auto& iblTexture = resourceManagers.mTextureManager.resolve(ibl->mConvolvedSkybox);
					resolvedShader.bindTexture("dfgLUT", resourceManagers.mTextureManager.resolve(ibl->mDFGLut));
					resolvedShader.bindTexture("ibl", iblTexture);
					resolvedShader.bindUniform("iblMips", iblTexture.mFormat.mMipCount - 1);
				}
			}

			const auto& drawSpatial = view.get<const SpatialComponent>(entity);
			resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
			resolvedShader.bindUniform("N", drawSpatial.getNormalMatrix());

			mesh.draw();
		}

		if (containsTransparency) {
			glDisable(GL_BLEND);
		}
	}
}