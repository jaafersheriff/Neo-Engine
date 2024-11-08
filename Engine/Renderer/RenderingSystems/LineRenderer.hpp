#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"

#include "Renderer/FrameGraph.hpp"

namespace neo {

	template<typename... CompTs, typename... Deps>
	void drawLines(
		FrameGraph& fg,
		FramebufferHandle outTarget,
		Viewport vp,
		const ResourceManagers& resourceManagers,
		ECS::Entity cameraEntity,
		const ShaderDefines& inDefines = {},
		Deps... deps
	) {
		TRACY_ZONE();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("LineShader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "line.vert"},
			{ types::shader::Stage::Fragment, "line.frag" }
			});

		ShaderDefines passDefines(inDefines);
		fg.pass(outTarget, vp, [passDefines, cameraEntity, shaderHandle](const ResourceManagers& resourceManagers, const ECS& ecs) mutable {
			TRACY_GPUN("Draw Lines");

			glEnable(GL_LINE_SMOOTH);

			auto& lineShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, passDefines);
			lineShader.bind();
			lineShader.bindUniform("P", ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj());
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
				line->getMesh(resourceManagers.mMeshManager).draw();
			}

			glEnable(GL_DEPTH_TEST);
		}, deps...);
	}
}
