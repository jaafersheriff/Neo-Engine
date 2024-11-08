#pragma once

#include "Util/Profiler.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Renderer/FrameGraph.hpp"

#include "Loader/Loader.hpp"
#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	template<typename... Deps>
	inline void blit(
		FrameGraph& fg,
		Viewport vp,
		const ResourceManagers& resourceManagers,
		FramebufferHandle srcHandle,
		FramebufferHandle dstHandle,
		uint8_t srcImage = 0,
		Deps... deps
	) {
		TRACY_ZONE();

		auto blitShaderHandle = resourceManagers.mShaderManager.asyncLoad("Blit Shader", SourceShader::ShaderCode {			
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

		fg.pass(dstHandle, vp, [srcHandle, dstHandle, srcImage, blitShaderHandle](const ResourceManagers& resourceManagers, const ECS&) {
			TRACY_GPU();
			if (!resourceManagers.mFramebufferManager.isValid(srcHandle) || !resourceManagers.mFramebufferManager.isValid(dstHandle)) {
				return;
			}
			const auto& src = resourceManagers.mFramebufferManager.resolve(srcHandle);

			if (src.mTextures.size() < srcImage || !resourceManagers.mTextureManager.isValid(src.mTextures[srcImage])) {
				return;
			}

			glDisable(GL_DEPTH_TEST);
			
			auto& resolvedBlit = resourceManagers.mShaderManager.resolveDefines(blitShaderHandle, {});
			
			// Bind input fbo texture
			resolvedBlit.bindTexture("inputTexture", resourceManagers.mTextureManager.resolve(src.mTextures[srcImage]));
			
			// Render 
			resourceManagers.mMeshManager.resolve("quad").draw();

			glEnable(GL_DEPTH_TEST);
		}, srcHandle, deps...);
	}
}