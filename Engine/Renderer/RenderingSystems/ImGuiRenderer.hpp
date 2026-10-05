#pragma once

#include "GLBackend/SourceShader.hpp"
#include "GLBackend/ResolvedShaderInstance.hpp"

#include "ECS/Component/RenderingComponent/ImGuiDrawComponent.hpp"

#include "ECS/ECS.hpp"

#include "Renderer/RenderingSystems/RenderPass.hpp"

#include "Util/Assert.hpp"

#include <glm/glm.hpp>

namespace neo {

	// One pass per ImGui draw, because each draw has its own scissor rect
	inline void drawImGui(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs, FramebufferHandle target, glm::uvec2 viewportOffset, glm::uvec2 viewportSize) {
		TRACY_ZONE();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("ImGuiShader", ShaderBuilder{}
			.setStage(types::shader::Stage::Vertex, "imgui.vert")
			.setStage(types::shader::Stage::Fragment, "imgui.frag")
		);

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

		RenderState renderState;
		renderState.mDepthState = std::nullopt;
		renderState.mCullFace = std::nullopt;
		renderState.mBlendState = BlendState{
			BlendEquation::Add,
			BlendFuncSrc::Alpha,
			BlendFuncDst::OneMinusSrcAlpha,
			glm::vec4(0.f),
			BlendFuncAlpha{ BlendFuncSrc::One, BlendFuncDst::OneMinusSrcAlpha }
		};
		renderState.mWireframeable = false;

		for(auto &&[_, draw, __]: ecs.getView<ImGuiDrawComponent, ImGuiComponent>().each()) {
			renderState.mScissor = glm::ivec4(
				draw.mScissorRect.x,
				viewportSize.y - draw.mScissorRect.y,
				draw.mScissorRect.z,
				draw.mScissorRect.w
			);

			// Unique render pass per draw call because scissor rects are applied at the pass-level :( 
			renderPasses.renderPass(target, viewportSize, renderState, [shaderHandle, ortho_projection, draw = draw](const ResourceManagers& resourceManagers, const ECS&) {
				if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
					return;
				}

				if (!resourceManagers.mMeshManager.isValid(draw.mMeshHandle)) {
					return;
				}

				if (!resourceManagers.mTextureManager.isValid(draw.mTextureView.mTextureHandle)) {
					return;
				}

				MakeDefine(TEXTURE_2D);
				MakeDefine(TEXTURE_2D_ARRAY);
				MakeDefine(TEXTURE_CUBE);
				MakeDefine(TEXTURE_3D);
				ShaderDefines drawDefines;

				const auto& resolvedTexture = resourceManagers.mTextureManager.resolve(draw.mTextureView.mTextureHandle);
				switch (resolvedTexture.mFormat.mTarget) {
				case types::texture::Target::Texture2D:
					drawDefines.set(TEXTURE_2D);
					break;
				case types::texture::Target::Texture2DArray:
					drawDefines.set(TEXTURE_2D_ARRAY);
					break;
				case types::texture::Target::TextureCube:
					drawDefines.set(TEXTURE_CUBE);
					break;
				case types::texture::Target::Texture3D:
					drawDefines.set(TEXTURE_3D);
					break;
				default:
					NEO_FAIL("ImGui::Image supplied with an unsupported texture target");
					return;
				}

				auto resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, drawDefines);
				resolvedShader.bindUniform("P", ortho_projection);
				resolvedShader.bindTexture("Texture", resolvedTexture);
				resolvedShader.bindUniform("arrayLevel", draw.mTextureView.mArrayLayer);
				resolvedShader.bindUniform("mipLevel", draw.mTextureView.mMipLevel);

				resourceManagers.mMeshManager.resolve(draw.mMeshHandle).draw(draw.mElementCount, draw.mElementBufferOffset, draw.mVertexOffset);
			}, "ImGui");
		}
	}
}
