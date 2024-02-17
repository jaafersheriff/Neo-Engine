#pragma once

#include "Texture.hpp"
#include "GLHelper.hpp"

#include "Util/Util.hpp"
#include "Util/Profiler.hpp"

#include "Renderer/Types.hpp"

#include <vector>

namespace neo {

	class Framebuffer {

	public:
		uint32_t mFBOID = 0;
		int mColorAttachments = 0;
		std::vector<Texture*> mTextures;

		void init();
		void bind();

		void disableDraw();
		void disableRead();

		void attachColorTexture(glm::uvec2 size, TextureFormat format);
		void attachDepthTexture(glm::ivec2 size, GLint format, GLint filter, GLenum mode);
		void attachDepthTexture(Texture* t);
		void attachStencilTexture(glm::uvec2 size, GLint filter, GLenum mode);
		void initDrawBuffers();
		void clear(glm::vec4 clearColor, GLbitfield clearFlags);
		void destroy();
	private:
		void _attachTexture(GLuint component, Texture& texture);
	};
}
