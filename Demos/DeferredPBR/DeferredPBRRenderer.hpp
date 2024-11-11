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
	void drawPointLightResolve(const ResourceManagers& resourceManagers, const ECS& ecs, const ECS::Entity cameraEntity, FramebufferHandle gbufferHandle, glm::uvec2 resolution, float debugRadius = 0.f) {
		TRACY_GPU();

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

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendColor(1.f, 1.f, 1.f, 1.f);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		/* Render light volumes */
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
			// TODO : Could do VFC
			MakeDefine(ENABLE_SHADOWS);
			const bool shadowsEnabled =
				ecs.has<ShadowCameraComponent>(entity)
				&& resourceManagers.mTextureManager.isValid(ecs.cGetComponent<ShadowCameraComponent>(entity)->mShadowMap);
			if (shadowsEnabled) {
				drawDefines.set(ENABLE_SHADOWS);
			}

			auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(lightResolveShaderHandle, drawDefines);
			resolvedShader.bind();

			const auto& light = ecs.cGetComponent<const LightComponent>(entity);
			const auto& spatial = ecs.cGetComponent<const SpatialComponent>(entity);
			resolvedShader.bindUniform("M", spatial->getModelMatrix());
			resolvedShader.bindUniform("lightPos", spatial->getPosition());
			resolvedShader.bindUniform("lightRadiance", glm::vec4(light->mColor, light->mIntensity));


			if (debugRadius > 0.f) {
				resolvedShader.bindUniform("debugRadius", debugRadius);
			}

			if (shadowsEnabled) {
				auto& shadowCube = resourceManagers.mTextureManager.resolve(ecs.cGetComponent<ShadowCameraComponent>(entity)->mShadowMap);
				resolvedShader.bindTexture("shadowCube", shadowCube);
				resolvedShader.bindUniform("shadowRange", static_cast<float>(spatial->getScale().x) / 2.f);
				resolvedShader.bindUniform("shadowMapResolution", static_cast<float>(shadowCube.mWidth));
			}

			resolvedShader.bindUniform("P", camera->getProj());
			resolvedShader.bindUniform("invP", glm::inverse(camera->getProj()));
			resolvedShader.bindUniform("V", cameraSpatial->getView());
			resolvedShader.bindUniform("invV", glm::inverse(cameraSpatial->getView()));
			resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());
			resolvedShader.bindUniform("resolution", glm::vec2(resolution));

			/* Bind gbuffer */
			resolvedShader.bindTexture("gAlbedoAO", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[0]));
			resolvedShader.bindTexture("gNormalRoughness", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[1]));
			resolvedShader.bindTexture("gEmissiveMetalness", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[2]));
			resolvedShader.bindTexture("gDepth", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[3]));

			// If camera is inside light 
			float dist = glm::distance(spatial->getPosition(), cameraSpatial->getPosition());
			if (dist - camera->getNear() < spatial->getScale().x) {
				glCullFace(GL_FRONT);
			}
			else {
				glCullFace(GL_BACK);
			}
			resourceManagers.mMeshManager.resolve(HashedString("sphere")).draw();
		}

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glCullFace(GL_BACK);
	}

	void drawIndirectResolve(const ResourceManagers& resourceManagers, const ECS& ecs, const ECS::Entity cameraEntity, FramebufferHandle gbufferHandle, std::optional<IBLComponent> ibl = std::nullopt) {
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

		glDisable(GL_DEPTH_TEST);
		resourceManagers.mMeshManager.resolve(HashedString("quad")).draw();
		glEnable(GL_DEPTH_TEST);
	}
}

