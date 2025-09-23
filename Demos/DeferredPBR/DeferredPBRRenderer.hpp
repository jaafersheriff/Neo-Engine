#pragma once

#include "Util/Profiler.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CSMCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/LightComponent/MainLightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

using namespace neo;

namespace DeferredPBR {

	template<typename... CompTs>
	void drawDirectionalLightResolve(
		RenderPasses& renderPasses,
		const FramebufferHandle& outputTargetHandle,
		const glm::uvec2& viewport,
		const ECS::Entity cameraEntity,
		const FramebufferHandle& gbufferHandle) {
		TRACY_ZONE();

		renderPasses.renderPass(outputTargetHandle, viewport, sDisableDepthState, [=](const ResourceManagers& resourceManagers, const ECS& ecs) {
			TRACY_GPU();
			if (!resourceManagers.mFramebufferManager.isValid(gbufferHandle)) {
				return;
			}

			auto lightResolveShaderHandle = resourceManagers.mShaderManager.asyncLoad("DirectionalLightResolveShader", SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "quad.vert" },
				{ types::shader::Stage::Fragment, "deferredpbr/directionallightresolve.frag" }
				});
			if (!resourceManagers.mShaderManager.isValid(lightResolveShaderHandle)) {
				return;
			}

			const auto& lightView = ecs.getView<DirectionalLightComponent, LightComponent, SpatialComponent, CompTs...>();
			ShaderDefines defines;
			for (auto& entity : lightView) {
				defines.reset();

				MakeDefine(ENABLE_SHADOWS);
				CSMShadowInfo csmShadowInfo = extractCSMShadowInfo(ecs, entity, resourceManagers.mTextureManager);
				if (csmShadowInfo.mValidCSMShadows) {
					defines.set(ENABLE_SHADOWS);
				}

				auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(lightResolveShaderHandle, defines);
				resolvedShader.bind();

				if (csmShadowInfo.mValidCSMShadows) {
					const auto& shadowMap = resourceManagers.mTextureManager.resolve(ecs.cGetComponent<CSMShadowMapComponent>(entity)->mShadowMap);
					resolvedShader.bindTexture("shadowMap", shadowMap);
					resolvedShader.bindUniform("shadowMapResolution", glm::vec2(shadowMap.mWidth, shadowMap.mHeight));
					resolvedShader.bindUniform("L0", csmShadowInfo.mLightArrays[0]);
					resolvedShader.bindUniform("L1", csmShadowInfo.mLightArrays[1]);
					resolvedShader.bindUniform("L2", csmShadowInfo.mLightArrays[2]);
				}

				const auto& camera = ecs.cGetComponent<CameraComponent>(cameraEntity);
				const auto& cameraSpatial = ecs.cGetComponent<const SpatialComponent>(cameraEntity);
				resolvedShader.bindUniform("P", camera->getProj());
				resolvedShader.bindUniform("invP", glm::inverse(camera->getProj()));
				resolvedShader.bindUniform("V", cameraSpatial->getView());
				resolvedShader.bindUniform("invV", glm::inverse(cameraSpatial->getView()));
				resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());

				/* Bind gbuffer */
				auto& gbuffer = resourceManagers.mFramebufferManager.resolve(gbufferHandle);
				resolvedShader.bindTexture("gAlbedoAO", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[0]));
				resolvedShader.bindTexture("gNormalRoughness", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[1]));
				resolvedShader.bindTexture("gEmissiveMetalness", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[2]));
				resolvedShader.bindTexture("gDepth", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[3]));

				const auto& light = ecs.cGetComponent<LightComponent>(entity);
				resolvedShader.bindUniform("lightRadiance", glm::vec4(light->mColor, light->mIntensity));
				resolvedShader.bindUniform("lightDir", -ecs.cGetComponent<SpatialComponent>(entity)->getLookDir());

				resourceManagers.mMeshManager.resolve(HashedString("quad")).draw();
			}
		}, "Directional Light Resolve");
	}

	template<typename... CompTs>
	void drawPointLightResolve(
		RenderPasses& renderPasses,
		const FramebufferHandle& outputTargetHandle,
		const glm::uvec2& viewport,
		const ECS::Entity cameraEntity,
		const FramebufferHandle& gbufferHandle,
		float debugRadius = 0.f
	) {
		TRACY_ZONE();

		auto drawFunc = [=](const ResourceManagers& resourceManagers, const ECS& ecs, bool drawInsideLights) {
			TRACY_GPUN("Point Light Resolve");

			auto lightResolveShaderHandle = resourceManagers.mShaderManager.asyncLoad("PointLightResolve Shader", SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "deferredpbr/pointlightresolve.vert"},
				{ types::shader::Stage::Fragment, "deferredpbr/pointlightresolve.frag" }
				});
			if (!resourceManagers.mShaderManager.isValid(lightResolveShaderHandle)) {
				return;
			}

			ShaderDefines passDefines;
			MakeDefine(SHOW_LIGHTS);
			if (debugRadius > 0.f) {
				passDefines.set(SHOW_LIGHTS);
			}

			const auto& camera = ecs.cGetComponent<CameraComponent>(cameraEntity);
			const auto& cameraSpatial = ecs.cGetComponent<const SpatialComponent>(cameraEntity);
			auto& gbuffer = resourceManagers.mFramebufferManager.resolve(gbufferHandle);

			// TODO : instanced

			// Sort them back to front first
			ecs.sort<PointLightComponent, SpatialComponent>([&cameraSpatial, &ecs](const ECS::Entity entityLeft, const ECS::Entity entityRight) {
				auto leftSpatial = ecs.cGetComponent<SpatialComponent>(entityLeft);
				auto rightSpatial = ecs.cGetComponent<SpatialComponent>(entityRight);
				if (leftSpatial && rightSpatial) {
					return glm::distance(cameraSpatial->getPosition(), leftSpatial->getPosition()) > glm::distance(cameraSpatial->getPosition(), rightSpatial->getPosition());
				}
				return false;
				});

			ShaderDefines drawDefines(passDefines);
			const auto& view = ecs.getView<const LightComponent, const PointLightComponent, const SpatialComponent, CompTs...>();
			for (auto entity : view) {
				const auto& light = ecs.cGetComponent<const LightComponent>(entity);
				const auto& spatial = ecs.cGetComponent<const SpatialComponent>(entity);

				// If camera is inside light 
				float dist = glm::distance(spatial->getPosition(), cameraSpatial->getPosition());
				bool lightInsideCamera = dist - camera->getNear() < spatial->getScale().x;
				if (drawInsideLights && !lightInsideCamera) {
					continue;
				}
				if (!drawInsideLights && lightInsideCamera) {
					continue;
				}

				// TODO : Could do VFC
				MakeDefine(ENABLE_SHADOWS);
				const bool shadowsEnabled =
					ecs.has<PointLightShadowMapComponent>(entity)
					&& resourceManagers.mTextureManager.isValid(ecs.cGetComponent<PointLightShadowMapComponent>(entity)->mShadowMap);
				if (shadowsEnabled) {
					drawDefines.set(ENABLE_SHADOWS);
				}

				auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(lightResolveShaderHandle, drawDefines);
				resolvedShader.bind();

				resolvedShader.bindUniform("M", spatial->getModelMatrix());
				resolvedShader.bindUniform("lightPos", spatial->getPosition());
				resolvedShader.bindUniform("lightRadiance", glm::vec4(light->mColor, light->mIntensity));

				if (debugRadius > 0.f) {
					resolvedShader.bindUniform("debugRadius", debugRadius);
				}

				if (shadowsEnabled) {
					auto& shadowCube = resourceManagers.mTextureManager.resolve(ecs.cGetComponent<PointLightShadowMapComponent>(entity)->mShadowMap);
					resolvedShader.bindTexture("shadowCube", shadowCube);
					resolvedShader.bindUniform("shadowRange", static_cast<float>(spatial->getScale().x) / 2.f);
					resolvedShader.bindUniform("shadowMapResolution", static_cast<float>(shadowCube.mWidth));
				}

				resolvedShader.bindUniform("P", camera->getProj());
				resolvedShader.bindUniform("invP", glm::inverse(camera->getProj()));
				resolvedShader.bindUniform("V", cameraSpatial->getView());
				resolvedShader.bindUniform("invV", glm::inverse(cameraSpatial->getView()));
				resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());
				resolvedShader.bindUniform("resolution", glm::vec2(viewport));

				/* Bind gbuffer */
				resolvedShader.bindTexture("gAlbedoAO", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[0]));
				resolvedShader.bindTexture("gNormalRoughness", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[1]));
				resolvedShader.bindTexture("gEmissiveMetalness", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[2]));
				resolvedShader.bindTexture("gDepth", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[3]));

				resourceManagers.mMeshManager.resolve(HashedString("sphere")).draw();
			}
		};

		RenderState renderState;
		renderState.mDepthState = std::nullopt;
		renderState.mBlendState = BlendState{
			BlendEquation::Add,
			BlendFuncSrc::One,
			BlendFuncDst::One,
			glm::vec4(1.f)
		};

		renderState.mCullFace = CullFace::Front;
		renderPasses.renderPass(outputTargetHandle, viewport, renderState, [=](const ResourceManagers& resourceManagers, const ECS& ecs) {
			return drawFunc(resourceManagers, ecs, true);
		}, "Point light resolve - Intersecting");

		renderState.mCullFace = CullFace::Back;
		renderPasses.renderPass(outputTargetHandle, viewport, renderState, [=](const ResourceManagers& resourceManagers, const ECS& ecs) {
			return drawFunc(resourceManagers, ecs, false);
		}, "Point light resolve");

	}

	void drawIndirectResolve(
		RenderPasses& renderPasses,
		const FramebufferHandle& outputTargetHandle,
		const glm::uvec2& viewport,
		const ECS::Entity cameraEntity,
		const FramebufferHandle& gbufferHandle,
		std::optional<IBLComponent> ibl = std::nullopt
	) {
		TRACY_ZONE();

		RenderState renderState;
		renderState.mDepthState = std::nullopt;
		renderState.mBlendState = BlendState{
			BlendEquation::Add,
			BlendFuncSrc::One,
			BlendFuncDst::One,
			glm::vec4(1.f)
		};

		renderPasses.renderPass(outputTargetHandle, viewport, renderState, [=](const ResourceManagers& resourceManagers, const ECS& ecs) {
			TRACY_GPU();

			if (!resourceManagers.mFramebufferManager.isValid(gbufferHandle)) {
				return;
			}

			auto lightResolveShaderHandle = resourceManagers.mShaderManager.asyncLoad("Indirect Resolve", SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "quad.vert" },
				{ types::shader::Stage::Fragment, "deferredpbr/indirectresolve.frag" }
				});
			if (!resourceManagers.mShaderManager.isValid(lightResolveShaderHandle)) {
				return;
			}

			ShaderDefines defines;
			MakeDefine(IBL);
			if (ibl.has_value()) {
				defines.set(IBL);
			}
			auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(lightResolveShaderHandle, defines);
			resolvedShader.bind();

			const auto& camera = ecs.cGetComponent<CameraComponent>(cameraEntity);
			const auto& cameraSpatial = ecs.cGetComponent<const SpatialComponent>(cameraEntity);
			resolvedShader.bindUniform("invP", glm::inverse(camera->getProj()));
			resolvedShader.bindUniform("invV", glm::inverse(cameraSpatial->getView()));
			resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());

			/* Bind gbuffer */
			auto& gbuffer = resourceManagers.mFramebufferManager.resolve(gbufferHandle);
			resolvedShader.bindTexture("gAlbedoAO", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[0]));
			resolvedShader.bindTexture("gNormalRoughness", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[1]));
			resolvedShader.bindTexture("gEmissiveMetalness", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[2]));
			resolvedShader.bindTexture("gDepth", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[3]));

			if (ibl.has_value()) {
				resolvedShader.bindTexture("ibl", resourceManagers.mTextureManager.resolve(ibl->mConvolvedSkybox));
				resolvedShader.bindTexture("dfgLUT", resourceManagers.mTextureManager.resolve(ibl->mDFGLut));
				resolvedShader.bindUniform("iblMips", resourceManagers.mTextureManager.resolve(ibl->mConvolvedSkybox).mFormat.mMipCount);
			}

			resourceManagers.mMeshManager.resolve(HashedString("quad")).draw();
		}, "Indirect Resolve");
	}
}

