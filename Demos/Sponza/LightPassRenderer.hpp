#pragma once

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/LightComponent/MainLightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "Renderer/GLObjects/Framebuffer.hpp"

using namespace neo;

namespace Sponza {
	void drawPointLights(const ResourceManagers& resourceManagers, const ECS& ecs, const Framebuffer& gbuffer, ECS::Entity cameraEntity, const glm::uvec2 resolution, const float debugRadius) {
		TRACY_GPU();

		auto lightResolveShaderHandle = resourceManagers.mShaderManager.asyncLoad("PointLightResolveShader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "sponza/pointlightresolve.vert"},
			{ types::shader::Stage::Fragment, "sponza/pointlightresolve.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(lightResolveShaderHandle)) {
			return;
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendColor(1.f, 1.f, 1.f, 1.f);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		ShaderDefines defines;
		MakeDefine(SHOW_LIGHTS);
		if (debugRadius > 0.f) {
			defines.set(SHOW_LIGHTS);
		}
		auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(lightResolveShaderHandle, defines);
		resolvedShader.bind();

		if (debugRadius > 0.f) {
			resolvedShader.bindUniform("debugRadius", debugRadius);
		}
		resolvedShader.bindUniform("resolution", glm::vec2(resolution));

		const auto& camera = ecs.cGetComponent<CameraComponent>(cameraEntity);
		const auto& cameraSpatial = ecs.cGetComponent<const SpatialComponent>(cameraEntity);
		resolvedShader.bindUniform("P", camera->getProj());
		resolvedShader.bindUniform("V", cameraSpatial->getView());
		resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());

		/* Bind gbuffer */
		resolvedShader.bindTexture("gAlbedo", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[0]));
		resolvedShader.bindTexture("gWorld", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[1]));
		resolvedShader.bindTexture("gNormal", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[2]));

		/* Render light volumes */
		// TODO : instanced
		const auto& view = ecs.getView<const LightComponent, const PointLightComponent, const SpatialComponent>();
		for (auto entity : view) {
			// TODO : Could do VFC
			const auto spatial = ecs.cGetComponent<const SpatialComponent>(entity);
			resolvedShader.bindUniform("M", spatial->getModelMatrix());
			resolvedShader.bindUniform("lightPos", spatial->getPosition());
			resolvedShader.bindUniform("lightRadius", spatial->getScale().x / 2.f);
			resolvedShader.bindUniform("lightCol", ecs.cGetComponent<const LightComponent>(entity)->mColor);

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

		// TODO - reset state
	}

	void drawDirectionalLights(const ResourceManagers& resourceManagers, const ECS& ecs, ECS::Entity cameraEntity, const Framebuffer& gbuffer, TextureHandle shadowMapHandle) {
		TRACY_GPU();

		auto lightResolveShader = resourceManagers.mShaderManager.asyncLoad("DirectionalLightResolveShader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "quad.vert"},
			{ types::shader::Stage::Fragment, "sponza/directionallightresolve.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(lightResolveShader)) {
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

		auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(lightResolveShader, defines);
		resolvedShader.bind();

		if (shadowsEnabled) {
			const auto& shadowMap = resourceManagers.mTextureManager.resolve(shadowMapHandle);
			resolvedShader.bindUniform("lightTransform", L);
			resolvedShader.bindTexture("shadowMap", shadowMap);
			resolvedShader.bindUniform("shadowMapResolution", glm::vec2(shadowMap.mWidth, shadowMap.mHeight));
		}

		auto&& [lightEntity, _lightLight, light, lightSpatial] = *ecs.getSingleView<MainLightComponent, LightComponent, SpatialComponent>();
		resolvedShader.bindUniform("lightDir", -lightSpatial.getOrientable().getLookDir());
		resolvedShader.bindUniform("lightCol", light.mColor);

		const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
		resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());

		/* Bind gbuffer */
		resolvedShader.bindTexture("gAlbedo", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[0]));
		resolvedShader.bindTexture("gWorld", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[1]));
		resolvedShader.bindTexture("gNormal", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[2]));

		glDisable(GL_DEPTH_TEST);

		resourceManagers.mMeshManager.resolve(HashedString("quad")).draw();

		// TODO - reset GL state
	}
}
