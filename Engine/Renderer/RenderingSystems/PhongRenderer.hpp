#pragma once

#include "ECS/ECS.hpp"
#include "Util/Profiler.hpp"

#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"
#include "ECS/Component/RenderingComponent/PhongRenderComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/TransparentComponent.hpp"
#include "ECS/Component/RenderingComponent/ShadowMapComponents.hpp"

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
	void drawPhong(
		RenderPasses& renderPasses, 
		const FramebufferHandle& outputTargetHandle, 
		const glm::uvec2 viewport, 
		const ECS::Entity cameraEntity
	) {
		TRACY_ZONE();

		constexpr bool containsAlphaTest = (std::is_same_v<AlphaTestComponent, CompTs> || ...);
		constexpr bool containsTransparency = (std::is_same_v<TransparentComponent, CompTs> || ...);

		RenderState renderState;
		if (containsTransparency) {
			renderState.mBlendState = BlendState{
				BlendEquation::Add,
				BlendFuncSrc::Alpha,
				BlendFuncDst::OneMinusSrcAlpha
			};
		}

		renderPasses.renderPass(outputTargetHandle, viewport, renderState, [=](const ResourceManagers& resourceManagers, const ECS& ecs) {
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

			ShaderDefines passDefines;
			MakeDefine(ALPHA_TEST);
			MakeDefine(TRANSPARENT);
			if (containsAlphaTest) {
				passDefines.set(ALPHA_TEST);
			}
			if (containsTransparency) {
				passDefines.set(TRANSPARENT);
			}

			const glm::mat4 P = ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj();
			const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
			auto&& [lightEntity, _lightLight, light, lightSpatial] = *ecs.getSingleView<MainLightComponent, LightComponent, SpatialComponent>();

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
					if (directionalLight) {
						resolvedShader.bindUniform("lightDir", -lightSpatial.getLookDir());
					}
					if (pointLight) {
						resolvedShader.bindUniform("lightPos", lightSpatial.getPosition());
						resolvedShader.bindUniform("lightRadiance", light.mIntensity);
					}
				}

				const auto& drawSpatial = view.get<const SpatialComponent>(entity);
				resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
				resolvedShader.bindUniform("N", drawSpatial.getNormalMatrix());

				resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle).draw();
			}
		}, "Draw Phong");
	}
}
