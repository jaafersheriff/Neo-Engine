#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"

namespace neo {

	template<typename... CompTs>
	void drawLines(const ResourceManagers& resourceManagers, const ECS& ecs, ECS::Entity cameraEntity, const ShaderDefines& inDefines = {}) {
		TRACY_GPU();

		auto lineShaderHandle = resourceManagers.mShaderManager.asyncLoad("LineShader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "line.vert"},
			{ types::shader::Stage::Fragment, "line.frag" }
			});
		if (!resourceManagers.mShaderManager.isValid(lineShaderHandle)) {
			return;
		}

		auto& lineShader = resourceManagers.mShaderManager.resolveDefines(lineShaderHandle, inDefines);
		lineShader.bind();
		lineShader.bindUniform("P", ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj());
		lineShader.bindUniform("V", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getView());

		const auto& view = ecs.getView<const LineMeshComponent, const SpatialComponent, CompTs...>();
		TRACY_GPUN("Draw view");

		for (auto entity : view) {
			const auto line = ecs.cGetComponent<const LineMeshComponent>(entity);

			glm::mat4 M(1.f);
			if (line->mUseParentSpatial) {
				if (auto spatial = ecs.cGetComponent<SpatialComponent>(entity)) {
					M = spatial->getModelMatrix();
				}
			}
			lineShader.bindUniform("M", M);

			line->getMesh(resourceManagers.mMeshManager).draw();
		}
	}
}
