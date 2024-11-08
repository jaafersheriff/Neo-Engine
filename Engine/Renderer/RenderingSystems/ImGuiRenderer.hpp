#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/Component/RenderingComponent/ImGuiDrawComponent.hpp"

#include "ECS/ECS.hpp"

#include "Util/Util.hpp"

#include <glm/glm.hpp>

namespace neo {

	template<typename... Deps>
	void drawImGui(
		FrameGraph& fg,
		FramebufferHandle outTarget,
		Viewport vp,
		const ResourceManagers& resourceManagers, 
		const ECS& ecs, 
		glm::uvec2 viewportOffset, 
		glm::uvec2 viewportSize,
		Deps... deps
	) {
		TRACY_ZONE();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("ImGuiShader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "imgui.vert"},
			{ types::shader::Stage::Fragment, "imgui.frag" }
		});

		{
			TRACY_ZONEN("Draw sorting");
			ecs.sort<ImGuiComponent, ImGuiDrawComponent>([&ecs](const ECS::Entity entityLeft, const ECS::Entity entityRight) {
				auto leftDraw = ecs.cGetComponent<ImGuiDrawComponent>(entityLeft);
				auto rightDraw = ecs.cGetComponent<ImGuiDrawComponent>(entityRight);
				if (leftDraw && rightDraw) {
					return leftDraw->mDrawOrder < rightDraw->mDrawOrder;
				}
				return false;
			});
		}

		fg.pass(outTarget, vp, [viewportOffset, viewportSize, shaderHandle](const ResourceManagers& resourceManagers, const ECS& ecs) {
			TRACY_GPU();
			float L = static_cast<float>(viewportOffset.x);
			float R = static_cast<float>(viewportOffset.x + viewportSize.x);
			float T = static_cast<float>(viewportOffset.y);
			float B = static_cast<float>(viewportOffset.y + viewportSize.y);
			const glm::mat4 ortho_projection = glm::mat4(
				2.0f / (R - L),   0.0f,         0.0f,   0.0f,
				0.0f,         2.0f / (T - B),   0.0f,   0.0f,
				0.0f,         0.0f,        -1.0f,   0.0f,
				(R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f
			);

			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_STENCIL_TEST);
			glEnable(GL_SCISSOR_TEST);
			auto resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, {});
			resolvedShader.bindUniform("P", ortho_projection);

			for (auto&& [_, draw, __] : ecs.getView<ImGuiDrawComponent, ImGuiComponent>().each()) {
				if (!resourceManagers.mMeshManager.isValid(draw.mMeshHandle)) {
					continue;
				}

				if (!resourceManagers.mTextureManager.isValid(draw.mTextureHandle)) {
					continue;
				}

				resolvedShader.bindTexture("Texture", resourceManagers.mTextureManager.resolve(draw.mTextureHandle));

				glScissor(
					draw.mScissorRect.x,
					viewportSize.y - draw.mScissorRect.y,
					draw.mScissorRect.z,
					draw.mScissorRect.w
				);

				resourceManagers.mMeshManager.resolve(draw.mMeshHandle).draw(draw.mElementCount, draw.mElementBufferOffset);
			}

			glDisable(GL_SCISSOR_TEST);
		}, deps...);
	}
}
