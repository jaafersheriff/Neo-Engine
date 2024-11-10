#pragma once

#include "ECS/ECS.hpp"
#include "Util/Profiler.hpp"

#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"
#include "ECS/Component/RenderingComponent/PhongRenderComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/TransparentComponent.hpp"

#include "ECS/Component/CameraComponent/ShadowCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
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
#include "Renderer/FrameGraph.hpp"

namespace neo {

	template<typename... CompTs, typename... Deps>
	void drawPhong(
		FrameGraph& fg,
		const Viewport& vp,
		const ResourceManagers& resourceManagers, 
		const ECS& ecs, 
		const ECS::Entity cameraEntity, 
		FramebufferHandle outhandle,
		Deps... deps
	) {
		TRACY_GPU();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("Phong Shader", 
			SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "model.vert"},
				{ types::shader::Stage::Fragment, "phong.frag" }
			}
		);

		const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
		PassState passState;
		if constexpr ((std::is_same_v<TransparentComponent, CompTs> || ...)) {
			passState.mBlending = true;
			passState.mBlendEquation = BlendEquation::Add;
			passState.mBlendSrcRGB = BlendFactor::Alpha;
			passState.mBlendSrcAlpha = BlendFactor::Alpha;
			passState.mBlendDstRGB = BlendFactor::OneMinusAlpha;
			passState.mBlendDstAlpha = BlendFactor::OneMinusAlpha;

			{
				TRACY_ZONEN("Transparency sorting");
				ecs.sort<PhongRenderComponent, TransparentComponent>([&cameraSpatial, &ecs](const ECS::Entity entityLeft, const ECS::Entity entityRight) {
					auto leftSpatial = ecs.cGetComponent<SpatialComponent>(entityLeft);
					auto rightSpatial = ecs.cGetComponent<SpatialComponent>(entityRight);
					if (leftSpatial && rightSpatial) {
						return glm::distance(cameraSpatial->getPosition(), leftSpatial->getPosition()) > glm::distance(cameraSpatial->getPosition(), rightSpatial->getPosition());
					}
					return false;
				});
			}
		}
		fg.pass(outhandle, vp, vp, passState, shaderHandle, [cameraEntity, cameraSpatial](Pass& pass, const ResourceManagers& resourceManagers, const ECS& ecs) {
			MakeDefine(ALPHA_TEST);
			MakeDefine(TRANSPARENT);
			if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
				pass.setDefine(ALPHA_TEST);
			}
			if constexpr ((std::is_same_v<TransparentComponent, CompTs> || ...)) {
				pass.setDefine(TRANSPARENT);
			}

			pass.bindUniform("P", ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj());
			pass.bindUniform("V", cameraSpatial->getView());
			pass.bindUniform("camPos", cameraSpatial->getPosition());

			auto&& [lightEntity, _lightLight, light, lightSpatial] = *ecs.getSingleView<MainLightComponent, LightComponent, SpatialComponent>();
			pass.bindUniform("lightCol", light.mColor);
			MakeDefine(DIRECTIONAL_LIGHT);
			MakeDefine(POINT_LIGHT);
			if (ecs.has<DirectionalLightComponent>(lightEntity)) {
				pass.setDefine(DIRECTIONAL_LIGHT);
				pass.bindUniform("lightDir", -lightSpatial.getLookDir());
			}
			else if (ecs.has<PointLightComponent>(lightEntity)) {
				pass.setDefine(POINT_LIGHT);
				pass.bindUniform("lightPos", lightSpatial.getPosition());
				pass.bindUniform("lightRadiance", light.mIntensity);
			}
			else {
				NEO_FAIL("Phong light needs a directional or point light component");
			}
			const bool shadowsEnabled =
				ecs.has<DirectionalLightComponent>(lightEntity)
				&& ecs.has<CameraComponent>(lightEntity)
				&& ecs.has<ShadowCameraComponent>(lightEntity)
				&& resourceManagers.mTextureManager.isValid(ecs.cGetComponent<ShadowCameraComponent>(lightEntity)->mShadowMap);
			MakeDefine(ENABLE_SHADOWS);
			if (shadowsEnabled) {
				pass.setDefine(ENABLE_SHADOWS);
				const auto& shadowCamera = *ecs.cGetComponent<CameraComponent>(lightEntity);
				static glm::mat4 biasMatrix(
					0.5f, 0.0f, 0.0f, 0.0f,
					0.0f, 0.5f, 0.0f, 0.0f,
					0.0f, 0.0f, 0.5f, 0.0f,
					0.5f, 0.5f, 0.5f, 1.0f);
				pass.bindUniform("lightDir", -lightSpatial.getLookDir());
				pass.bindUniform("L", biasMatrix * shadowCamera.getProj() * lightSpatial.getView());
				pass.bindTexture("shadowMap", ecs.cGetComponent<ShadowCameraComponent>(lightEntity)->mShadowMap);
			}


			const auto& view = ecs.getView<const PhongRenderComponent, const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
			for (auto entity : view) {
				// VFC
				if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
					if (!culled->isInView(ecs, entity, cameraEntity)) {
						continue;
					}
				}

				ShaderDefinesFG drawDefines;
				UBO ubo;

				const auto& material = view.get<const MaterialComponent>(entity);
				MakeDefine(ALBEDO_MAP);
				MakeDefine(NORMAL_MAP);

				ubo.bindUniform("albedo", material.mAlbedoColor);
				if (resourceManagers.mTextureManager.isValid(material.mAlbedoMap)) {
					drawDefines.set(ALBEDO_MAP);
					ubo.bindTexture("albedoMap", material.mAlbedoMap);
				}
				if (resourceManagers.mTextureManager.isValid(material.mNormalMap)) {
					drawDefines.set(NORMAL_MAP);
					ubo.bindTexture("normalMap", material.mNormalMap);
				}

				const auto& drawSpatial = view.get<const SpatialComponent>(entity);
				ubo.bindUniform("M", drawSpatial.getModelMatrix());
				ubo.bindUniform("N", drawSpatial.getNormalMatrix());

				pass.drawCommand(view.get<const MeshComponent>(entity).mMeshHandle, std::move(ubo), std::move(drawDefines));
			}
			})
			.mDebugName = "Phong";

	}
}
