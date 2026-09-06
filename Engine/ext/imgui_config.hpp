
#pragma once


#include "Util/Util.hpp"
#define IM_ASSERT(_EXPR) do { NEO_ASSERT(_EXPR, "ImGui Failed"); } while (0)
#define IM_DEBUG_BREAK() do { NEO_FAIL("ImGui Failed"); } while (0)

// Stored in every ImDrawCmd, try to keep it small
namespace ImGui {
	struct TextureView {
		std::uint32_t mTextureHandle = 0; // matches TextureHandle -> ResourceHandle<>::mHandle -> HashedString::hash_type
		uint32_t mArrayLayer = 0;
		uint32_t mMipLevel = 0;

		TextureView() noexcept = default;
		TextureView(std::uint32_t handle) noexcept
			: mTextureHandle(handle) {}
		TextureView(std::uint32_t handle, uint32_t arrayLayer, uint32_t mipLevel) noexcept
			: mTextureHandle(handle), mArrayLayer(arrayLayer), mMipLevel(mipLevel) {}

		operator intptr_t() const noexcept {
			return static_cast<intptr_t>(mTextureHandle);
		}

		bool operator==(const TextureView& other) const noexcept {
			return mTextureHandle == other.mTextureHandle
				&& mArrayLayer == other.mArrayLayer
				&& mMipLevel == other.mMipLevel;
		}
		bool operator!=(const TextureView& other) const noexcept {
			return !(*this == other);
		}
	};
}

#define ImTextureID ImGui::TextureView

