#pragma once

#include "ECS/ECS.hpp"
#include "Util/Profiler.hpp"

#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"
#include "ECS/Component/RenderingComponent/PhongRenderComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/TransparentComponent.hpp"

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

namespace CSM {

	using namespace neo;

	template<typename... CompTs>
	void drawCSMResolve(const ResourceManagers& resourceManagers, const ECS& ecs, const ECS::Entity cameraEntity, const ShaderDefines& inDefines = {}) {
		TRACY_GPU();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("CSM Resolve Shader", 
			SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "csm/model.vert"},
				{ types::shader::Stage::Fragment, "csm/csm.frag" }
			}
		);
		if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
			return;
		}

		ShaderDefines passDefines(inDefines);
		bool containsAlphaTest = false;
		bool containsTransparency = false;
		MakeDefine(ALPHA_TEST);
		MakeDefine(TRANSPARENT);
		if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
			containsAlphaTest = true;
			passDefines.set(ALPHA_TEST);
		}
		if constexpr ((std::is_same_v<TransparentComponent, CompTs> || ...)) {
			containsTransparency = true;
			passDefines.set(TRANSPARENT);

			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		const glm::mat4 P = ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj();
		const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
		auto lightView = ecs.getSingleView<MainLightComponent, LightComponent, SpatialComponent>();
		if (!lightView) {
			return;
		}
		const auto& [lightEntity, ____, light, lightSpatial] = *lightView;

		glm::mat4 L, L0, L1, L2, L3;
		const bool shadowsEnabled = 
			ecs.has<DirectionalLightComponent>(lightEntity) 
			&& ecs.has<CameraComponent>(lightEntity) 
			&& ecs.has<CSMShadowMapComponent>(lightEntity) 
			&& resourceManagers.mTextureManager.isValid(ecs.cGetComponent<CSMShadowMapComponent>(lightEntity)->mShadowMap);
		MakeDefine(ENABLE_SHADOWS);
		if (shadowsEnabled) {
			passDefines.set(ENABLE_SHADOWS);
			const auto& shadowCamera = *ecs.cGetComponent<CameraComponent>(lightEntity);
			static glm::mat4 biasMatrix(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				0.5f, 0.5f, 0.5f, 1.0f);
			L = biasMatrix * shadowCamera.getProj() * lightSpatial.getView();
			// TODO - this should have asserts
			if (auto csmCamera0 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera0>()) {
				L0 = biasMatrix * std::get<2>(*csmCamera0).getProj() * std::get<1>(*csmCamera0).getView();
			}
			if (auto csmCamera1 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera1>()) {
				L1 = biasMatrix * std::get<2>(*csmCamera1).getProj() * std::get<1>(*csmCamera1).getView();
			}
			if (auto csmCamera2 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera2>()) {
				L2 = biasMatrix * std::get<2>(*csmCamera2).getProj() * std::get<1>(*csmCamera2).getView();
			}
			if (auto csmCamera3 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera3>()) {
				L3 = biasMatrix * std::get<2>(*csmCamera3).getProj() * std::get<1>(*csmCamera3).getView();
			}
		}
		bool directionalLight = ecs.has<DirectionalLightComponent>(lightEntity);
		bool pointLight = ecs.has<PointLightComponent>(lightEntity);
		MakeDefine(DIRECTIONAL_LIGHT);
		MakeDefine(POINT_LIGHT);
		if (directionalLight) {
			passDefines.set(DIRECTIONAL_LIGHT);
		}
		else if (pointLight) {
			passDefines.set(POINT_LIGHT);
		}
		else {
			NEO_FAIL("Phong light needs a directional or point light component");
		}

		ShaderDefines drawDefines(passDefines);
		// No transparency sorting on the view, because I'm lazy, and this is stinky phong renderer
		const auto& view = ecs.getView<const PhongRenderComponent, const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
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

			if (resourceManagers.mTextureManager.isValid(material.mAlbedoMap)) {
				drawDefines.set(ALBEDO_MAP);
			}
			if (resourceManagers.mTextureManager.isValid(material.mNormalMap)) {
				drawDefines.set(NORMAL_MAP);
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

			// UBO candidates
			{
				resolvedShader.bindUniform("P", P);
				resolvedShader.bindUniform("V", cameraSpatial->getView());
				resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());
				resolvedShader.bindUniform("lightCol", light.mColor);
				if (directionalLight || shadowsEnabled) {
					resolvedShader.bindUniform("lightDir", -lightSpatial.getLookDir());
				}
				if (pointLight) {
					resolvedShader.bindUniform("lightPos", lightSpatial.getPosition());
					resolvedShader.bindUniform("lightRadiance", light.mIntensity);
				}
				if (shadowsEnabled) {
					resolvedShader.bindUniform("L", L);
					// These could be an array tbh
					resolvedShader.bindUniform("L0", L0);
					resolvedShader.bindUniform("L1", L1);
					resolvedShader.bindUniform("L2", L2);
					resolvedShader.bindUniform("L3", L3);
					resolvedShader.bindTexture("shadowMap", resourceManagers.mTextureManager.resolve(ecs.cGetComponent<CSMShadowMapComponent>(lightEntity)->mShadowMap));
				}
			}

			const auto& drawSpatial = view.get<const SpatialComponent>(entity);
			resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
			resolvedShader.bindUniform("N", drawSpatial.getNormalMatrix());

			resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle).draw();
		}

		if (containsTransparency) {
			glDisable(GL_BLEND);
		}

	}
}