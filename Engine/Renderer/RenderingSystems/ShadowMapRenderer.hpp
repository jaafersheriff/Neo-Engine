#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

namespace neo {

	template<typename... CompTs>
	void drawShadows(const ResourceManagers& resourceManagers, const ECS& ecs, ECS::Entity lightEntity) {
		TRACY_GPU();
		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("ShadowMap Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "model.vert"},
			{ types::shader::Stage::Fragment, "depth.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
			return;
		}

		glCullFace(GL_FRONT);

		bool containsAlphaTest = false;
		if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...) || (std::is_same_v<TransparentComponent, CompTs> || ...)) {
			// TODO - set GL state?
			containsAlphaTest = true;
		}

		NEO_ASSERT(ecs.has<SpatialComponent>(lightEntity) && ecs.has<CameraComponent>(lightEntity), "Invalid light for shadow draws");
		const glm::mat4 P = ecs.cGetComponent<CameraComponent>(lightEntity)->getProj();
		const glm::mat4 V = ecs.cGetComponent<SpatialComponent>(lightEntity)->getView();

		ShaderDefines drawDefines;
		const auto& view = ecs.getView<const ShadowCasterRenderComponent, const MeshComponent, const SpatialComponent, CompTs...>();
		for (auto entity : view) {
			// VFC
			if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
				if (!culled->isInView(ecs, entity, lightEntity)) {
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

			resolvedShader.bindUniform("P", P);
			resolvedShader.bindUniform("V", V);
			resolvedShader.bindUniform("M", view.get<const SpatialComponent>(entity).getModelMatrix());
			resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle).draw();
		}

		glCullFace(GL_BACK);
	}
}
