
#pragma once


#include "Util/Util.hpp"
#define IM_ASSERT(_EXPR) do { NEO_ASSERT(_EXPR, "ImGui Failed"); } while (0)
#define IM_DEBUG_BREAK() do { NEO_FAIL("ImGui Failed"); } while (0)

namespace ImGui {
	struct TextureDescriptor {
		std::uint32_t mTextureHandle; // matches entt::id_type
		uint32_t mArrayLayer = 0;
		uint32_t mMipLevel = 0;

		TextureDescriptor(int handle) noexcept
            : mTextureHandle(static_cast<std::uint32_t>(handle)) {}

        TextureDescriptor() noexcept = default;

		operator intptr_t() const noexcept {
			return static_cast<intptr_t>(mTextureHandle);
		}

		bool operator==(const TextureDescriptor& other) const noexcept {
			return mTextureHandle == other.mTextureHandle &&
				   mArrayLayer	== other.mArrayLayer &&
				   mMipLevel	 == other.mMipLevel;
		}

		bool operator!=(const TextureDescriptor& other) const noexcept {
			return !(*this == other);
		}
	};
}

#define ImTextureID ImGui::TextureDescriptor

