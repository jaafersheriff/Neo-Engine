#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"

namespace neo {

	template<typename... CompTs>
	void drawLines(const MeshManager& meshManager, const ECS& ecs, ECS::Entity cameraEntity, const ShaderDefines& inDefines = {}) {
		TRACY_GPU();

		bool oldDepthState = glIsEnabled(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);

		const auto& lineShader = Library::createSourceShader("LineShader", SourceShader::ConstructionArgs{
			{ ShaderStage::VERTEX, "line.vert"},
			{ ShaderStage::FRAGMENT, "line.frag" }
		})->getResolvedInstance(inDefines);

		lineShader.bind();
		lineShader.bindUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
		lineShader.bindUniform("V", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getView());

		const auto& view = ecs.getView<const LineMeshComponent, const SpatialComponent, CompTs...>();
		for (auto entity : view) {
			const auto line = ecs.cGetComponent<const LineMeshComponent>(entity);

			glm::mat4 M(1.f);
			if (line->mUseParentSpatial) {
				if (auto spatial = ecs.cGetComponent<SpatialComponent>(entity)) {
					M = spatial->getModelMatrix();
				}
			}
			lineShader.bindUniform("M", M);
			if (line->mWriteDepth) {
				glEnable(GL_DEPTH_TEST);
			}
			else {
				glDisable(GL_DEPTH_TEST);
			}

			/* Bind mesh */
			meshManager.get(line->mMeshHandle).draw();
		}

		if (oldDepthState) {
			glEnable(GL_DEPTH_TEST);
		}
		else {
			glDisable(GL_DEPTH_TEST);
		}
	}
}
