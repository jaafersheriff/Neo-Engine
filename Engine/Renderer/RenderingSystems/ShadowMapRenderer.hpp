#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

namespace neo {

	template<typename... CompTs, typename... Deps>
	void drawShadows(
		FrameGraph& fg,
		FramebufferHandle outputTarget,
		const ResourceManagers& resourceManagers,
		ECS::Entity lightEntity,
		Deps... deps
	) {
		TRACY_ZONE();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("ShadowMap Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "model.vert"},
			{ types::shader::Stage::Fragment, "depth.frag" }
			});

		Viewport vp(0);
		{
			if (!resourceManagers.mFramebufferManager.isValid(outputTarget)) {
				return;
			}
			TextureHandle shadowTextureHandle = resourceManagers.mFramebufferManager.resolve(outputTarget).mTextures[0];
			if (!resourceManagers.mTextureManager.isValid(shadowTextureHandle)) {
				return;
			}
			const Texture& shadowTexture = resourceManagers.mTextureManager.resolve(shadowTextureHandle);
			vp.z = shadowTexture.mWidth;
			vp.w = shadowTexture.mHeight;
		}

		PassState passState;
		passState.mCullFace = true;
		passState.mCullOrder = types::passState::CullOrder::Front;
		fg.pass(outputTarget, vp, vp, passState, shaderHandle)
			.with([lightEntity](Pass& pass, const ResourceManagers& resourceManagers, const ECS& ecs) {
			TRACY_ZONEN("drawShadows PassBuilder");
			NEO_ASSERT(ecs.has<DirectionalLightComponent>(lightEntity) && ecs.has<ShadowCameraComponent>(lightEntity), "Invalid light entity");
			NEO_ASSERT(ecs.has<SpatialComponent>(lightEntity) && ecs.has<CameraComponent>(lightEntity), "Light entity is just wrong");
			const glm::mat4 P = ecs.cGetComponent<CameraComponent>(lightEntity)->getProj();
			const glm::mat4 V = ecs.cGetComponent<SpatialComponent>(lightEntity)->getView();

			const auto& view = ecs.getView<const ShadowCasterRenderComponent, const MeshComponent, const SpatialComponent, CompTs...>();
			for (auto entity : view) {
				// VFC
				if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
					if (!culled->isInView(ecs, entity, lightEntity)) {
						continue;
					}
				}
				ShaderDefinesFG drawDefines;
				UniformBuffer uniforms;

				auto material = ecs.cGetComponent<const MaterialComponent>(entity);

				MakeDefine(ALPHA_TEST);
				if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
					if (material && resourceManagers.mTextureManager.isValid(material->mAlbedoMap)) {
						drawDefines.set(ALPHA_TEST);
						uniforms.bindTexture("alphaMap", material->mAlbedoMap);
					}
				}

				uniforms.bindUniform("P", P);
				uniforms.bindUniform("V", V);
				uniforms.bindUniform("M", view.get<const SpatialComponent>(entity).getModelMatrix());
				pass.drawCommand(view.get<const MeshComponent>(entity).mMeshHandle, uniforms, drawDefines);
			}
		})
			.dependsOn(resourceManagers, std::forward<Deps>(deps)...)
			.setDebugName("Draw Shadows");
	}
}
