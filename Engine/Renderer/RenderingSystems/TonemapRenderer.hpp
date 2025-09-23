#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	inline TextureHandle tonemap(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, glm::uvec2 dimension, TextureHandle inputTextureHandle, TextureHandle averageLuminance = NEO_INVALID_HANDLE) {
		TRACY_ZONE();

		if (!resourceManagers.mTextureManager.isValid(inputTextureHandle)) {
			return NEO_INVALID_HANDLE;
		}

		auto tonemapTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"Tonemapped",
			FramebufferBuilder{}
				.setSize(dimension)
				.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGB8_UNORM }),
			resourceManagers.mTextureManager
		);
		renderPasses.clear(tonemapTargetHandle, types::framebuffer::AttachmentBit::Color, glm::vec4(0.f, 0.f, 0.f, 1.f), "Clear tonemap target");

		renderPasses.renderPass(tonemapTargetHandle, dimension, sBlitRenderState, [averageLuminance, inputTextureHandle](const ResourceManagers& resourceManagers, const ECS&) {
			TRACY_GPU();
			auto tonemapShaderHandle = resourceManagers.mShaderManager.asyncLoad("Tonemap Shader", SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "quad.vert"},
				{ types::shader::Stage::Fragment, "tonemap.frag" }
				});
			if (!resourceManagers.mShaderManager.isValid(tonemapShaderHandle)) {
				return;
			}

			ShaderDefines defines;
			MakeDefine(AUTO_EXPOSURE);
			if (resourceManagers.mTextureManager.isValid(averageLuminance)) {
				defines.set(AUTO_EXPOSURE);
			}

			auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(tonemapShaderHandle, defines);
			resolvedShader.bind();

			auto& inputTexture = resourceManagers.mTextureManager.resolve(inputTextureHandle);
			resolvedShader.bindTexture("inputTexture", inputTexture);
			if (resourceManagers.mTextureManager.isValid(averageLuminance)) {
				resolvedShader.bindTexture("averageLuminance", resourceManagers.mTextureManager.resolve(averageLuminance));
			}

			resourceManagers.mMeshManager.resolve("quad").draw();
		}, "Tonemap");

		if (resourceManagers.mFramebufferManager.isValid(tonemapTargetHandle)) {
			return resourceManagers.mFramebufferManager.resolve(tonemapTargetHandle).mTextures[0];
		}

		return NEO_INVALID_HANDLE;
	}
}
