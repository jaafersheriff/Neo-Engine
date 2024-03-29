#pragma once

#include "Util/Profiler.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Loader/Library.hpp"
#include "Loader/Loader.hpp"
#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	static void blit(ResourceManagers& resourceManagers, Framebuffer& outputFBO, Texture& inputTexture, glm::uvec2 viewport, glm::vec4 clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f)) {
		TRACY_GPU();

		auto blitShaderHandle = resourceManagers.mShaderManager.asyncLoad("Blit Shader", SourceShader::ShaderCode {			
			{ ShaderStage::VERTEX,
			R"(
				layout (location = 0) in vec3 vertPos;
				layout (location = 2) in vec2 vertTex;
				out vec2 fragTex;
				void main() { 
					gl_Position = vec4(2 * vertPos, 1); 
					fragTex = vertTex; 
				} 
			)"},
			{ ShaderStage::FRAGMENT,
			R"(
				in vec2 fragTex;
				layout (binding = 0) uniform sampler2D inputTexture;
				out vec4 color;
				void main() {
					color = texture(inputTexture, fragTex);
				}
			)"}
		}); 

		outputFBO.bind();
		glViewport(0, 0, viewport.x, viewport.y);
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		glClear(GL_COLOR_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		
		auto& resolvedBlit = resourceManagers.mShaderManager.resolveDefines(blitShaderHandle, {});
		
		// Bind input fbo texture
		resolvedBlit.bindTexture("inputTexture", inputTexture);
		
		// Render 
		resourceManagers.mMeshManager.resolve("quad").draw();

		glEnable(GL_DEPTH_TEST);
	}
}