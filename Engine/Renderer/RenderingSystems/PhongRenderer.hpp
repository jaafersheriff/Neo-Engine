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

namespace neo {

	template<typename... CompTs>
	void drawPhong(const ResourceManagers& resourceManagers, const ECS& ecs, const ECS::Entity cameraEntity, TextureHandle shadowMapHandle = NEO_INVALID_HANDLE, const ShaderDefines& inDefines = {}) {
		TRACY_GPU();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("Phong Shader", 
			SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "model.vert"},
				{ types::shader::Stage::Fragment, "phong.frag" }
			}
		);
		if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
			return;
		}

		ShaderDefines passDefines(inDefines);
		bool containsAlphaTest = false;
		bool containsTransparency = false;
		MakeDefine(ALPHA_TEST);
		MakeDefine(TRANSPARENT)
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
		auto&& [lightEntity, _lightLight, light, lightSpatial] = *ecs.getSingleView<MainLightComponent, LightComponent, SpatialComponent>();

		glm::mat4 L;
		const auto shadowCamera = ecs.getSingleView<ShadowCameraComponent, CameraComponent, SpatialComponent>();
		const bool shadowsEnabled = resourceManagers.mTextureManager.isValid(shadowMapHandle) && shadowCamera.has_value();
		MakeDefine(ENABLE_SHADOWS);
		if (shadowsEnabled) {
			passDefines.set(ENABLE_SHADOWS);
			const auto& [_, __, shadowOrtho, shadowCameraSpatial] = *shadowCamera;
			static glm::mat4 biasMatrix(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				0.5f, 0.5f, 0.5f, 1.0f);
			L = biasMatrix * shadowOrtho.getProj() * shadowCameraSpatial.getView();
		}
		bool directionalLight = ecs.has<DirectionalLightComponent>(lightEntity);
		bool pointLight = ecs.has<PointLightComponent>(lightEntity);
		glm::vec3 attenuation(0.f);
		MakeDefine(DIRECTIONAL_LIGHT);
		MakeDefine(POINT_LIGHT);
		if (directionalLight) {
			passDefines.set(DIRECTIONAL_LIGHT);
		}
		else if (pointLight) {
			attenuation = ecs.cGetComponent<PointLightComponent>(lightEntity)->mAttenuation;
			passDefines.set(POINT_LIGHT);
		}
		else {
			NEO_FAIL("Phong light needs a directional or point light component");
		}

		ShaderDefines drawDefines(passDefines);
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
					resolvedShader.bindUniform("lightAtt", attenuation);
				}
				if (shadowsEnabled) {
					resolvedShader.bindUniform("L", L);
					resolvedShader.bindTexture("shadowMap", resourceManagers.mTextureManager.resolve(shadowMapHandle));
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
