#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/Component/RenderingComponent/ImGuiDrawComponent.hpp"

#include "ECS/ECS.hpp"

#include "Util/Util.hpp"

namespace neo {

	void drawImGui(const ResourceManagers& resourceManagers, const ECS& ecs, glm::uvec2 viewportOffset, glm::uvec2 viewportSize) {
		TRACY_GPU();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("ImGuiShader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "imgui.vert"},
			{ types::shader::Stage::Fragment, "imgui.frag" }
			});
		auto imguiDraws = ecs.getView<ImGuiDrawComponent>(); // TODO - this will break

		if (!imguiDraws.size() || !resourceManagers.mShaderManager.isValid(shaderHandle)) {
			return;
		}

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glEnable(GL_SCISSOR_TEST);

		float R = static_cast<float>(viewportOffset.x + viewportSize.x);
		float L = static_cast<float>(viewportOffset.x);
		float T = static_cast<float>(viewportOffset.y);
		float B = static_cast<float>(viewportOffset.y + viewportSize.y);
		const float ortho_projection[4][4] =
		{
			{ 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
			{ 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
			{ 0.0f,         0.0f,        -1.0f,   0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
		};


		imguiDraws.each([&](ECS::Entity, ImGuiDrawComponent& draw) {
			if (!resourceManagers.mMeshManager.isValid(draw.mMeshHandle)) {
				return;
			}
			if (!resourceManagers.mTextureManager.isValid(draw.mTextureHandle)) {
				return;
			}

			auto resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, {});
			resolvedShader.bindUniform("P", ortho_projection);
			resolvedShader.bindTexture("Texture", resourceManagers.mTextureManager.resolve(draw.mTextureHandle));

			glScissor(
				draw.mScissorRect.x,
				viewportSize.y - draw.mScissorRect.w,
				draw.mScissorRect.z - draw.mScissorRect.x,
				draw.mScissorRect.w - draw.mScissorRect.y
			);

			resourceManagers.mMeshManager.resolve(draw.mMeshHandle).draw(draw.mElementCount, draw.mElementBufferOffset);
		});
	}
}
