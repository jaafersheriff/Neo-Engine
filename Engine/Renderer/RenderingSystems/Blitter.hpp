#pragma once

#include "Util/Profiler.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/RenderingSystems/RenderPass.hpp"

#include "Loader/Loader.hpp"
#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	inline void blit(const ResourceManagers& resourceManagers, TextureHandle inputTextureHandle) {
		TRACY_GPU();

		auto blitShaderHandle = resourceManagers.mShaderManager.asyncLoad("Blit Shader", SourceShader::ShaderCode{
			{ types::shader::Stage::Vertex,
			R"(
				layout (location = 0) in vec3 vertPos;
				layout (location = 2) in vec2 vertTex;
				out vec2 fragTex;
				void main() { 
					gl_Position = vec4(2 * vertPos, 1); 
					fragTex = vertTex; 
				} 
			)"},
			{ types::shader::Stage::Fragment,
			R"(
				in vec2 fragTex;
				layout (binding = 0) uniform sampler2D inputTexture;
				out vec4 color;
				void main() {
					color = texture(inputTexture, fragTex);
				}
			)"}
			});
		if (!resourceManagers.mShaderManager.isValid(blitShaderHandle)) {
			return; // RIP
		}

		if (!resourceManagers.mTextureManager.isValid(inputTextureHandle)) {
			return;
		}

		glDisable(GL_DEPTH_TEST);

		auto& resolvedBlit = resourceManagers.mShaderManager.resolveDefines(blitShaderHandle, {});

		// Bind input fbo texture
		resolvedBlit.bindTexture("inputTexture", resourceManagers.mTextureManager.resolve(inputTextureHandle));

		// Render 
		resourceManagers.mMeshManager.resolve("quad").draw();
	}

	inline void blitDepth(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, TextureHandle srcTexture, TextureHandle dstTexture, glm::uvec2 dimension) {
		TRACY_ZONE();

		FramebufferHandle inputTarget = resourceManagers.mFramebufferManager.asyncLoad("Input Depth Blit",
			FramebufferExternalAttachments{
				FramebufferAttachment{srcTexture},
			},
			resourceManagers.mTextureManager
		);
		FramebufferHandle outputTarget = resourceManagers.mFramebufferManager.asyncLoad("Output Depth Blit",
			FramebufferExternalAttachments{
				FramebufferAttachment{dstTexture},
			},
			resourceManagers.mTextureManager
		);

		// use compute pass to avoid useless/invalid overhead of renderpasses
		renderPasses.computePass([inputTarget, outputTarget, dimension](const ResourceManagers& resourceManagers, const ECS&) {
			if (resourceManagers.mFramebufferManager.isValid(inputTarget) && resourceManagers.mFramebufferManager.isValid(outputTarget)) {
				const Framebuffer& inputFramebuffer = resourceManagers.mFramebufferManager.resolve(inputTarget);
				const Framebuffer& outputFramebuffer = resourceManagers.mFramebufferManager.resolve(outputTarget);
				glBlitNamedFramebuffer(inputFramebuffer.mFBOID, outputFramebuffer.mFBOID,
					0, 0, dimension.x, dimension.y,
					0, 0, dimension.x, dimension.y,
					GL_DEPTH_BUFFER_BIT,
					GL_NEAREST
				);
			}
		}, "Depth blit");


	}
}
