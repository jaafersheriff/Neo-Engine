#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/RenderingSystems/RenderPass.hpp"

#include "ECS/Component/RenderingComponent/WireframeRenderComponent.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	// Declares its own pass rather than drawing inline. It used to disable culling and swap the polygon
	// mode with raw GL calls, then put both back - which only worked because nothing else in the pass
	// cared. Both are render state now, so the pass says what it needs and applyRenderState sets it.
	template<typename... CompTs>
	void drawWireframe(FramebufferHandle outputHandle, glm::uvec2 viewport, RenderPasses& renderPasses, ECS::Entity cameraEntity) {
		RenderState renderState;
		renderState.mCullFace = std::nullopt;
		renderState.mPolygonMode = PolygonMode::Line;

		renderPasses.renderPass(outputHandle, viewport, renderState,
			[cameraEntity](const ResourceManagers& resourceManagers, const ECS& ecs) {
				TRACY_GPU();

				auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("Wireframe Shader", ShaderBuilder{}
					.setStage(types::shader::Stage::Vertex, "model.vert")
					.setStage(types::shader::Stage::Fragment, "color.frag")
				);
				if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
					return;
				}

				const auto& view = ecs.getView<const WireframeRenderComponent, const MeshComponent, const SpatialComponent, CompTs...>();
				for (auto entity : view) {
					// VFC
					if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
						if (!culled->isInView(ecs, entity, cameraEntity)) {
							continue;
						}
					}

					auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, {});
					resolvedShader.bind();

					resolvedShader.bindUniform("P", ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj());
					resolvedShader.bindUniform("V", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getView());
					resolvedShader.bindUniform("M", view.get<const SpatialComponent>(entity).getModelMatrix());
					resolvedShader.bindUniform("color", view.get<const WireframeRenderComponent>(entity).mColor);

					resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle).draw();
				}
			}, "Draw wireframe");
	}
}
