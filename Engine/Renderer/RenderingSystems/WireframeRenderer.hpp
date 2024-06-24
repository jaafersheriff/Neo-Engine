#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/Component/RenderingComponent/WireframeRenderComponent.hpp"

namespace neo {

	template<typename... CompTs>
	void drawWireframe(const ResourceManagers& resourceManagers, const ECS& ecs, ECS::Entity cameraEntity, const ShaderDefines& inDefines = {}) {
		TRACY_GPU();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("Wireframe Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "model.vert"},
			{ types::shader::Stage::Fragment, "color.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
			return;
		}

		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		const auto& view = ecs.getView<const WireframeRenderComponent, const MeshComponent, const SpatialComponent, CompTs...>();
		for (auto entity : view) {
			// VFC
			if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
				if (!culled->isInView(ecs, entity, cameraEntity)) {
					continue;
				}
			}

			auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, inDefines);
			resolvedShader.bind();

			resolvedShader.bindUniform("P", ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj());
			resolvedShader.bindUniform("V", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getView());
			resolvedShader.bindUniform("M", view.get<const SpatialComponent>(entity).getModelMatrix());
			resolvedShader.bindUniform("color", view.get<const WireframeRenderComponent>(entity).mColor);

			resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle).draw();
		}

		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}
