#pragma once

#include "Util/Profiler.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/ShadowCameraComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/LightComponent/MainLightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/FrameGraph/FrameGraph.hpp"

using namespace neo;

namespace DeferredPBR {

	template<typename... CompTs, typename... Deps>
	inline void drawDirectionalLightResolve(
		FrameGraph& fg,
		FramebufferHandle outputHandle,
		Viewport vp,
		const ResourceManagers& resourceManagers, 
		const ECS& ecs,
		const ECS::Entity cameraEntity, 
		FramebufferHandle gbufferHandle,
		Deps... deps
	) {
		TRACY_GPU();

		if (!resourceManagers.mFramebufferManager.isValid(gbufferHandle)) {
			return;
		}

		auto lightResolveShaderHandle = resourceManagers.mShaderManager.asyncLoad("DirectionalLightResolveShader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "quad.vert" },
			{ types::shader::Stage::Fragment, "deferredpbr/directionallightresolve.frag" }
		});

		PassState passState;
		passState.mDepthTest = false;

		TextureHandle shadowMapHandle = NEO_INVALID_HANDLE;
		const auto lightView = ecs.getSingleView<LightComponent, SpatialComponent, CompTs...>();
		if (!lightView) {
			return;
		}

		// This could all go in a for loop tbh
		const ECS::Entity& lightEntity = std::get<0>(*lightView);
		const LightComponent& light = std::get<1>(*lightView);
		const SpatialComponent& lightSpatial = std::get<2>(*lightView);
		const bool shadowsEnabled =
			ecs.has<DirectionalLightComponent>(lightEntity)
			&& ecs.has<CameraComponent>(lightEntity)
			&& ecs.has<ShadowCameraComponent>(lightEntity)
			&& resourceManagers.mTextureManager.isValid(ecs.cGetComponent<ShadowCameraComponent>(lightEntity)->mShadowMap)
			;

		fg.pass(outputHandle, vp, vp, passState, lightResolveShaderHandle)
			.with([cameraEntity, gbufferHandle, lightEntity, light, lightSpatial, shadowsEnabled](Pass& pass, const ResourceManagers& resourceManagers, const ECS& ecs) {
			TRACY_ZONEN("Directional light resolve");
			MakeDefine(ENABLE_SHADOWS);
			if (shadowsEnabled) {
				pass.setDefine(ENABLE_SHADOWS);
				static glm::mat4 biasMatrix(
					0.5f, 0.0f, 0.0f, 0.0f,
					0.0f, 0.5f, 0.0f, 0.0f,
					0.0f, 0.0f, 0.5f, 0.0f,
					0.5f, 0.5f, 0.5f, 1.0f);
				pass.bindUniform("lightTransform", biasMatrix * ecs.cGetComponent<CameraComponent>(lightEntity)->getProj() * lightSpatial.getView());
				pass.bindTexture("shadowMap", ecs.cGetComponent<ShadowCameraComponent>(lightEntity)->mShadowMap);
				const Texture& shadowTexture = resourceManagers.mTextureManager.resolve(ecs.cGetComponent<ShadowCameraComponent>(lightEntity)->mShadowMap);
				pass.bindUniform("shadowMapResolution", glm::vec2(shadowTexture.mWidth, shadowTexture.mHeight));
			}

			const auto& camera = ecs.cGetComponent<CameraComponent>(cameraEntity);
			const auto& cameraSpatial = ecs.cGetComponent<const SpatialComponent>(cameraEntity);
			pass.bindUniform("P", camera->getProj());
			pass.bindUniform("invP", glm::inverse(camera->getProj()));
			pass.bindUniform("V", cameraSpatial->getView());
			pass.bindUniform("invV", glm::inverse(cameraSpatial->getView()));
			pass.bindUniform("camPos", cameraSpatial->getPosition());

			/* Bind gbuffer */
			auto& gbuffer = resourceManagers.mFramebufferManager.resolve(gbufferHandle);
			pass.bindTexture("gAlbedoAO", gbuffer.mTextures[0]);
			pass.bindTexture("gNormalRoughness", gbuffer.mTextures[1]);
			pass.bindTexture("gEmissiveMetalness", gbuffer.mTextures[2]);
			pass.bindTexture("gDepth", gbuffer.mTextures[3]);

			pass.bindUniform("lightRadiance", glm::vec4(light.mColor, light.mIntensity));
			pass.bindUniform("lightDir", -lightSpatial.getLookDir());

			pass.drawCommand(MeshHandle("quad"), {}, {});
		})
			.dependsOn(resourceManagers, gbufferHandle, shadowsEnabled ? ecs.cGetComponent<ShadowCameraComponent>(lightEntity)->mShadowMap : NEO_INVALID_HANDLE, std::forward<Deps>(deps)...)
			.setDebugName("DirectionalLightResolve");
	}

	template<typename... CompTs>
	void drawPointLightResolve(
		FrameGraph& fg,
		FramebufferHandle outTarget,
		Viewport vp,
		const ResourceManagers& resourceManagers,
		const ECS& ecs,
		const ECS::Entity cameraEntity,
		FramebufferHandle gbufferHandle,
		float debugRadius = 0.f
	) {
		TRACY_ZONE();

		if (!resourceManagers.mFramebufferManager.isValid(gbufferHandle)) {
			return;
		}

		auto lightResolveShaderHandle = resourceManagers.mShaderManager.asyncLoad("PointLightResolve Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "deferredpbr/pointlightresolve.vert"},
			{ types::shader::Stage::Fragment, "deferredpbr/pointlightresolve.frag" }
			});

		const auto& camera = ecs.cGetComponent<CameraComponent>(cameraEntity);
		const auto& cameraSpatial = ecs.cGetComponent<const SpatialComponent>(cameraEntity);

		// Sort them back to front first
		ecs.sort<PointLightComponent, SpatialComponent>([&cameraSpatial, &ecs](const ECS::Entity entityLeft, const ECS::Entity entityRight) {
			auto leftSpatial = ecs.cGetComponent<SpatialComponent>(entityLeft);
			auto rightSpatial = ecs.cGetComponent<SpatialComponent>(entityRight);
			if (leftSpatial && rightSpatial) {
				return glm::distance(cameraSpatial->getPosition(), leftSpatial->getPosition()) > glm::distance(cameraSpatial->getPosition(), rightSpatial->getPosition());
			}
			return false;
			});

		// Inidividual passes to set up dependencies wahoo
		const auto& view = ecs.getView<const LightComponent, const PointLightComponent, const SpatialComponent, CompTs...>();
		for (auto entity : view) {
			PassState passState;
			passState.mBlending = true;
			passState.mBlendSrcRGB = passState.mBlendSrcAlpha = passState.mBlendDstRGB = passState.mBlendDstAlpha = types::passState::BlendFactor::One;
			//glBlendColor(1.f, 1.f, 1.f, 1.f);
			passState.mDepthTest = false;
			passState.mCullFace = true;

			const auto& light = view.get<const LightComponent>(entity);
			const auto& spatial = view.get<const SpatialComponent>(entity);

			// If camera is inside light 
			float dist = glm::distance(spatial.getPosition(), cameraSpatial->getPosition());
			if (dist - camera->getNear() < spatial.getScale().x) {
				passState.mCullOrder = types::passState::CullOrder::Front;
			}
			else {
				passState.mCullOrder = types::passState::CullOrder::Back;
			}

			const bool shadowsEnabled =
				ecs.has<ShadowCameraComponent>(entity)
				&& resourceManagers.mTextureManager.isValid(ecs.cGetComponent<ShadowCameraComponent>(entity)->mShadowMap);

			fg.pass(outTarget, vp, vp, passState, lightResolveShaderHandle)
				.with([debugRadius, camera, cameraSpatial, vp, shadowsEnabled, entity, light, spatial, gbufferHandle](Pass& pass, const ResourceManagers& resourceManagers, const ECS& ecs) {
				TRACY_ZONEN("Point Light Resolve");
				MakeDefine(SHOW_LIGHTS);
				if (debugRadius > 0.f) {
					pass.setDefine(SHOW_LIGHTS);
					pass.bindUniform("debugRadius", debugRadius);
				}
				pass.bindUniform("P", camera->getProj());
				pass.bindUniform("invP", glm::inverse(camera->getProj()));
				pass.bindUniform("V", cameraSpatial->getView());
				pass.bindUniform("invV", glm::inverse(cameraSpatial->getView()));
				pass.bindUniform("camPos", cameraSpatial->getPosition());
				pass.bindUniform("resolution", glm::vec2(vp.z, vp.w));

				// TODO : Could do VFC
				MakeDefine(ENABLE_SHADOWS);
				if (shadowsEnabled) {
					pass.setDefine(ENABLE_SHADOWS);
					auto& shadowCube = resourceManagers.mTextureManager.resolve(ecs.cGetComponent<ShadowCameraComponent>(entity)->mShadowMap);
					pass.bindTexture("shadowCube", ecs.cGetComponent<ShadowCameraComponent>(entity)->mShadowMap);
					pass.bindUniform("shadowRange", static_cast<float>(spatial.getScale().x) / 2.f);
					pass.bindUniform("shadowMapResolution", static_cast<float>(shadowCube.mWidth));
				}

				pass.bindUniform("M", spatial.getModelMatrix());
				pass.bindUniform("lightPos", spatial.getPosition());
				pass.bindUniform("lightRadiance", glm::vec4(light.mColor, light.mIntensity));

				/* Bind gbuffer */
				auto& gbuffer = resourceManagers.mFramebufferManager.resolve(gbufferHandle);
				pass.bindTexture("gAlbedoAO", gbuffer.mTextures[0]);
				pass.bindTexture("gNormalRoughness", gbuffer.mTextures[1]);
				pass.bindTexture("gEmissiveMetalness", gbuffer.mTextures[2]);
				pass.bindTexture("gDepth", gbuffer.mTextures[3]);

				// TODO : instanced
				pass.drawCommand(HashedString("sphere"), {}, {});
					})
				.dependsOn(resourceManagers, gbufferHandle, shadowsEnabled ? ecs.cGetComponent<ShadowCameraComponent>(entity)->mShadowMap : NEO_INVALID_HANDLE)
				.setDebugName("Point Light Resolve");
		}
	}

	void drawIndirectResolve(const ResourceManagers& resourceManagers, const ECS& ecs, const ECS::Entity cameraEntity, FramebufferHandle gbufferHandle, std::optional<IBLComponent> ibl = std::nullopt) {
		NEO_UNUSED(resourceManagers, ecs, cameraEntity, gbufferHandle, ibl);
//		TRACY_GPU();
//
//		if (!resourceManagers.mFramebufferManager.isValid(gbufferHandle)) {
//			return;
//		}
//
//		auto lightResolveShaderHandle = resourceManagers.mShaderManager.asyncLoad("Indirect Resolve", SourceShader::ConstructionArgs{
//			{ types::shader::Stage::Vertex, "quad.vert" },
//			{ types::shader::Stage::Fragment, "deferredpbr/indirectresolve.frag" }
//			});
//		if (!resourceManagers.mShaderManager.isValid(lightResolveShaderHandle)) {
//			return;
//		}
//
//		ShaderDefines defines;
//		MakeDefine(IBL);
//		if (ibl.has_value()) {
//			defines.set(IBL);
//		}
//		auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(lightResolveShaderHandle, defines);
//		resolvedShader.bind();
//
//		const auto& camera = ecs.cGetComponent<CameraComponent>(cameraEntity);
//		const auto& cameraSpatial = ecs.cGetComponent<const SpatialComponent>(cameraEntity);
//		resolvedShader.bindUniform("invP", glm::inverse(camera->getProj()));
//		resolvedShader.bindUniform("invV", glm::inverse(cameraSpatial->getView()));
//		resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());
//
//		/* Bind gbuffer */
//		auto& gbuffer = resourceManagers.mFramebufferManager.resolve(gbufferHandle);
//		resolvedShader.bindTexture("gAlbedoAO", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[0]));
//		resolvedShader.bindTexture("gNormalRoughness", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[1]));
//		resolvedShader.bindTexture("gEmissiveMetalness", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[2]));
//		resolvedShader.bindTexture("gDepth", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[3]));
//
//		if (ibl.has_value()) {
//			resolvedShader.bindTexture("ibl", resourceManagers.mTextureManager.resolve(ibl->mConvolvedSkybox));
//			resolvedShader.bindTexture("dfgLUT", resourceManagers.mTextureManager.resolve(ibl->mDFGLut));
//			resolvedShader.bindUniform("iblMips", resourceManagers.mTextureManager.resolve(ibl->mConvolvedSkybox).mFormat.mMipCount);
//		}
//
//		glDisable(GL_DEPTH_TEST);
//		resourceManagers.mMeshManager.resolve(HashedString("quad")).draw();
//		glEnable(GL_DEPTH_TEST);
	}
}

