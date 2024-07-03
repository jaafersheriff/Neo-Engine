#pragma once

#include "Util/Profiler.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Loader/Loader.hpp"
#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	inline void blit(const ResourceManagers& resourceManagers, const Framebuffer& outputFBO, TextureHandle inputTextureHandle, glm::uvec2 viewport, glm::vec4 clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f)) {
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

		outputFBO.bind();
		glViewport(0, 0, viewport.x, viewport.y);
		outputFBO.clear(clearColor, types::framebuffer::AttachmentBit::Color);

		glDisable(GL_DEPTH_TEST);
		
		auto& resolvedBlit = resourceManagers.mShaderManager.resolveDefines(blitShaderHandle, {});
		
		// Bind input fbo texture
		resolvedBlit.bindTexture("inputTexture", resourceManagers.mTextureManager.resolve(inputTextureHandle));
		
		// Render 
		resourceManagers.mMeshManager.resolve("quad").draw();

		glEnable(GL_DEPTH_TEST);
	}
}