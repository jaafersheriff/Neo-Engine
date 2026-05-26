#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/Component/RenderingComponent/ImGuiDrawComponent.hpp"

#include "ECS/ECS.hpp"

#include "Util/Util.hpp"

#include <glm/glm.hpp>

namespace neo {

	void drawImGui(const ResourceManagers& resourceManagers, const ECS& ecs, glm::uvec2 viewportOffset, glm::uvec2 viewportSize) {
		TRACY_GPU();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("ImGuiShader", ShaderBuilder{}
			.setStage(types::shader::Stage::Vertex, "imgui.vert")
			.setStage(types::shader::Stage::Fragment, "imgui.frag")
		);

		if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
			return;
		}


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

		// I'm too lazy to translate these into some Neo interface thing
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ZERO, GL_ZERO);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glEnable(GL_SCISSOR_TEST);

		MakeDefine(TEXTURE_2D);
		MakeDefine(TEXTURE_2D_ARRAY);
		MakeDefine(TEXTURE_CUBE);
		MakeDefine(TEXTURE_3D);
		ShaderDefines drawDefines;
		for(auto &&[_, draw, __]: ecs.getView<ImGuiDrawComponent, ImGuiComponent>().each()) {
			if (!resourceManagers.mMeshManager.isValid(draw.mMeshHandle)) {
				return;
			}

			if (!resourceManagers.mTextureManager.isValid(draw.mTextureDescriptor.mTextureHandle)) {
				return;
			}
			drawDefines.reset();

			const auto& resolvedTexture = resourceManagers.mTextureManager.resolve(draw.mTextureDescriptor.mTextureHandle);
			if (resolvedTexture.mFormat.mTarget == types::texture::Target::Texture2D) {
				drawDefines.set(TEXTURE_2D);
			}
			else if (resolvedTexture.mFormat.mTarget == types::texture::Target::Texture2DArray) {
				drawDefines.set(TEXTURE_2D_ARRAY);
			}
			else if (resolvedTexture.mFormat.mTarget == types::texture::Target::TextureCube) {
				drawDefines.set(TEXTURE_CUBE);
			}
			else if (resolvedTexture.mFormat.mTarget == types::texture::Target::Texture3D) {
				drawDefines.set(TEXTURE_3D);
			}
			else {
				NEO_FAIL("ImGui::Image supplied w/ unsupported texture type");
				return;
			}

			auto resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, drawDefines);
			resolvedShader.bindUniform("P", ortho_projection);
			resolvedShader.bindTexture("Texture", resourceManagers.mTextureManager.resolve(draw.mTextureDescriptor.mTextureHandle));
			resolvedShader.bindUniform("arrayLevel", draw.mTextureDescriptor.mArrayLayer);
			resolvedShader.bindUniform("mipLevel", draw.mTextureDescriptor.mMipLevel);

			glScissor(
				draw.mScissorRect.x,
				viewportSize.y - draw.mScissorRect.y,
				draw.mScissorRect.z,
				draw.mScissorRect.w
			);

			resourceManagers.mMeshManager.resolve(draw.mMeshHandle).draw(draw.mElementCount, draw.mElementBufferOffset);
		}

		glDisable(GL_SCISSOR_TEST);
	}
}
