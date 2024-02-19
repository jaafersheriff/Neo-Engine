#pragma once

#include "Texture.hpp"
#include "GLHelper.hpp"

#include "Util/Util.hpp"
#include "Util/Profiler.hpp"

#include "Renderer/Types.hpp"

#include <vector>

namespace neo {
	// The biggest hax
	enum class ClearFlagBits : uint8_t {
		Color = 1 << 0,
		Depth = 1 << 1,
		Stencil = 1 << 2,
	};
	inline ClearFlagBits operator|(ClearFlagBits a, ClearFlagBits b) {
		return static_cast<ClearFlagBits>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
	}
	struct ClearFlags {
		uint8_t mClearFlagBits = 0;
		ClearFlags(uint8_t bits) {
			mClearFlagBits = bits;
		}
		ClearFlags(ClearFlagBits bits) {
			mClearFlagBits = static_cast<uint8_t>(bits);
		}
		inline ClearFlags operator|(ClearFlagBits b) {
			return ClearFlags(static_cast<uint8_t>(b));
		}
	};

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
		void attachDepthTexture(glm::ivec2 size, types::texture::InternalFormats format, TextureFilter filter, TextureWrap wrap);
		void attachDepthTexture(Texture* t);
		void attachStencilTexture(glm::uvec2 size, TextureFilter filter, TextureWrap wrap);
		void initDrawBuffers();
		void clear(glm::vec4 clearColor, ClearFlags clearFlags);
		void destroy();
	};
}
