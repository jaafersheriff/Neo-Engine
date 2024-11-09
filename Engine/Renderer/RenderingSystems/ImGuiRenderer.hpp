#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/Component/RenderingComponent/ImGuiDrawComponent.hpp"

#include "ECS/ECS.hpp"

#include "Util/Util.hpp"

#include <glm/glm.hpp>

namespace neo {

#pragma warning(push)
	#pragma warning (disable : 4100 )
	template<typename... Deps>
	inline void drawImGui(
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

		PassState passState;
		passState.mBlending = true;
		passState.mBlendEquation = BlendEquation::Add;
		passState.mBlendSrcRGB = BlendFactor::Alpha;
		passState.mBlendDstRGB = BlendFactor::OneMinusAlpha;
		passState.mBlendSrcAlpha = BlendFactor::One;
		passState.mBlendDstAlpha = BlendFactor::OneMinusAlpha;
		passState.mCullFace = false;
		passState.mDepthTest = false;
		passState.mStencilTest = false;
		passState.mScissorTest = true;
		float L = static_cast<float>(viewportOffset.x);
		float R = static_cast<float>(viewportOffset.x + viewportSize.x);
		float T = static_cast<float>(viewportOffset.y);
		float B = static_cast<float>(viewportOffset.y + viewportSize.y);
		const glm::mat4 ortho_projection = glm::mat4(
			2.0f / (R - L), 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f / (T - B), 0.0f, 0.0f,
			0.0f, 0.0f, -1.0f, 0.0f,
			(R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f
		);

		for (auto&& [_, draw, __] : ecs.getView<ImGuiDrawComponent, ImGuiComponent>().each()) {

			// Need an individual pass per draw b/c scissor..........
			Viewport scissor(
				draw.mScissorRect.x,
				viewportSize.y - draw.mScissorRect.y,
				draw.mScissorRect.z,
				draw.mScissorRect.w

			);
			fg.pass(outTarget, vp, scissor, passState, shaderHandle, [draw, ortho_projection, viewportOffset, viewportSize](Pass& pass, const ResourceManagers& resourceManagers, const ECS& ecs) {
				pass.bindUniform("P", ortho_projection);
				pass.bindTexture("Texture", draw.mTextureHandle);

				pass.drawCommand(draw.mMeshHandle, {}, {}, draw.mElementCount, draw.mElementBufferOffset);
			}, deps...);
		}
	}
#pragma warning(pop)
}
