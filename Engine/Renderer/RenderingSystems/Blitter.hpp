#pragma once

#include "Util/Profiler.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Loader/Loader.hpp"
#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	inline void blit(const ResourceManagers& resourceManagers, TextureHandle inputTextureHandle) {
		TRACY_GPU();

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
		if (!resourceManagers.mShaderManager.isValid(blitShaderHandle)) {
			return; // RIP
		}

		if (!resourceManagers.mTextureManager.isValid(inputTextureHandle)) {
			return;
		}

		glDisable(GL_DEPTH_TEST);
		int oldPolygonMode;
		glGetIntegerv(GL_POLYGON_MODE, &oldPolygonMode);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		auto& resolvedBlit = resourceManagers.mShaderManager.resolveDefines(blitShaderHandle, {});
		
		// Bind input fbo texture
		resolvedBlit.bindTexture("inputTexture", resourceManagers.mTextureManager.resolve(inputTextureHandle));
		
		// Render 
		resourceManagers.mMeshManager.resolve("quad").draw();

		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, oldPolygonMode);
	}

	// inline void blitDepth(const ResourceManagers& resourceManagers, TextureHandle inputTextureHandle) {
	// 	// typedef void (GLAPIENTRY * PFNGLBLITNAMEDFRAMEBUFFERPROC) (GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
	// 	glBlitNamedFramebuffer(sceneTarget.mFBOID, backbuffer.mFBOID,
	// 		0, 0, viewport.mSize.x, viewport.mSize.y,
	// 		0, 0, viewport.mSize.x, viewport.mSize.y,
	// 		GL_DEPTH_BUFFER_BIT,
	// 		GL_NEAREST
	// 	);
	// }

}