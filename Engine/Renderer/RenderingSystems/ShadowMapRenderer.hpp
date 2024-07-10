#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

namespace neo {

	template<typename... CompTs>
	void drawShadows(const ResourceManagers& resourceManagers, const Framebuffer& depthMap, const ECS& ecs) {
		TRACY_GPU();
		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("ShadowMap Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "model.vert"},
			{ types::shader::Stage::Fragment, "depth.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
			return;
		}

		depthMap.disableDraw();
		auto& depthTexture = resourceManagers.mTextureManager.resolve(depthMap.mTextures[0]);
		glViewport(0, 0, depthTexture.mWidth, depthTexture.mHeight);
		{
			glBindTexture(GL_TEXTURE_2D, depthTexture.mTextureID);
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, std::vector<float>{1.f, 1.f, 1.f, 1.f}.data());
		}

		glCullFace(GL_FRONT);

		bool containsAlphaTest = false;
		if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...) || (std::is_same_v<TransparentComponent, CompTs> || ...)) {
			// TODO - set GL state?
			containsAlphaTest = true;
		}

		auto shadowCameraView = ecs.getSingleView<DirectionalLightComponent, ShadowCameraComponent, CameraComponent, SpatialComponent>();
		if (!shadowCameraView) {
			NEO_ASSERT(shadowCameraView, "No shadow camera found");
		}
		auto&& [shadowCameraEntity, _, __, shadowCamera, shadowCameraSpatial] = *shadowCameraView;

		ShaderDefines drawDefines;
		const auto& view = ecs.getView<const ShadowCasterRenderComponent, const MeshComponent, const SpatialComponent, CompTs...>();
		for (auto entity : view) {
			// VFC
			if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
				if (!culled->isInView(ecs, entity, shadowCameraEntity)) {
					continue;
				}
			}
			drawDefines.reset();

			auto material = ecs.cGetComponent<const MaterialComponent>(entity);

			MakeDefine(ALPHA_TEST);
			bool doAlphaTest = containsAlphaTest && material && resourceManagers.mTextureManager.isValid(material->mAlbedoMap);
			if (doAlphaTest) {
				drawDefines.set(ALPHA_TEST);
			}

			auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, drawDefines);
			resolvedShader.bind();

			if (doAlphaTest) {
				resolvedShader.bindTexture("alphaMap", resourceManagers.mTextureManager.resolve(material->mAlbedoMap));
			}

			resolvedShader.bindUniform("P", shadowCamera.getProj());
			resolvedShader.bindUniform("V", shadowCameraSpatial.getView());
			resolvedShader.bindUniform("M", view.get<const SpatialComponent>(entity).getModelMatrix());
			resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle).draw();
		}

		glCullFace(GL_BACK);
	}
}
