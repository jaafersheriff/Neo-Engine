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

using namespace neo;

namespace PBR {

	template<typename... CompTs>
	void drawDirectionalLightResolve(const ResourceManagers& resourceManagers, const ECS& ecs, const ECS::Entity cameraEntity, FramebufferHandle gbufferHandle, TextureHandle shadowMapHandle = NEO_INVALID_HANDLE) {
		TRACY_GPU();

		if (!resourceManagers.mFramebufferManager.isValid(gbufferHandle)) {
			return;
		}

		auto lightResolveShaderHandle = resourceManagers.mShaderManager.asyncLoad("DirectionalLightResolveShader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "quad.vert" },
			{ types::shader::Stage::Fragment, "pbr/directionallightresolve.frag" }
			});
		if (!resourceManagers.mShaderManager.isValid(lightResolveShaderHandle)) {
			return;
		}

		ShaderDefines defines;
		glm::mat4 L;
		const auto shadowCamera = ecs.getSingleView<ShadowCameraComponent, CameraComponent, SpatialComponent>();
		const bool shadowsEnabled = resourceManagers.mTextureManager.isValid(shadowMapHandle) && shadowCamera.has_value();
		MakeDefine(ENABLE_SHADOWS);
		if (shadowsEnabled) {
			defines.set(ENABLE_SHADOWS);
			const auto& [_, __, shadowOrtho, shadowCameraSpatial] = *shadowCamera;
			static glm::mat4 biasMatrix(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				0.5f, 0.5f, 0.5f, 1.0f);
			L = biasMatrix * shadowOrtho.getProj() * shadowCameraSpatial.getView();
		}

		auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(lightResolveShaderHandle, defines);
		resolvedShader.bind();

		if (shadowsEnabled) {
			const auto& shadowMap = resourceManagers.mTextureManager.resolve(shadowMapHandle);
			resolvedShader.bindUniform("lightTransform", L);
			resolvedShader.bindTexture("shadowMap", shadowMap);
			resolvedShader.bindUniform("shadowMapResolution", glm::vec2(shadowMap.mWidth, shadowMap.mHeight));
		}

		resolvedShader.bindUniform("camPos", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getPosition());

		/* Bind gbuffer */
		auto& gbuffer = resourceManagers.mFramebufferManager.resolve(gbufferHandle);
		resolvedShader.bindTexture("gAlbedoAO", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[0]));
		resolvedShader.bindTexture("gNormal", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[1]));
		resolvedShader.bindTexture("gWorldRoughness", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[2]));
		resolvedShader.bindTexture("gEmissiveMetalness", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[3]));
		resolvedShader.bindTexture("gDepth", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[4]));

		const auto& lightView = ecs.getView<LightComponent, SpatialComponent, CompTs...>();
		for (auto& entity : lightView) {
			const auto& light = ecs.cGetComponent<LightComponent>(entity);
			resolvedShader.bindUniform("lightRadiance", glm::vec4(light->mColor, light->mIntensity));
			resolvedShader.bindUniform("lightDir", -ecs.cGetComponent<SpatialComponent>(entity)->getLookDir());

			glDisable(GL_DEPTH_TEST);
			resourceManagers.mMeshManager.resolve(HashedString("quad")).draw();
			glEnable(GL_DEPTH_TEST);
		}
	}

	void drawPointLightResolve() {
		TRACY_GPU();

		NEO_LOG_E("Unimplemented");
	}
}

