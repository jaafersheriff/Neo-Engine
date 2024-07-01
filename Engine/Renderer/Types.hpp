#pragma once

#include <stdint.h>

namespace neo {
	namespace types {
		enum class ByteFormats : uint8_t {
			UnsignedByte,
			Byte,
			Short,
			UnsignedShort,
			Int,
			UnsignedInt,
			UnsignedInt24_8,
			Double,
			Float
		};

		namespace shader {
			enum class Stage {
				Vertex,
				Fragment,
				Geometry,
				TessellationControl,
				TessellationEval,
				Compute
			};

			enum class Barrier {
				ImageAccess,
				AtomicCounter,
				StorageBuffer,
				// There's way more that are unsupported hehe
			};

			enum class Access {
				Ready,
				Write,
				ReadWrite
			};
		}

		namespace framebuffer {
			enum class AttachmentBit : uint8_t {
				Color = 1 << 0,
				Depth = 1 << 1,
				Stencil = 1 << 2,
			};
			inline AttachmentBit operator|(AttachmentBit a, AttachmentBit b) {
				return static_cast<AttachmentBit>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
			}
			struct AttachmentBits {
				uint8_t mClearBits = 0;
				AttachmentBits(uint8_t bits) {
					mClearBits = bits;
				}
				AttachmentBits(AttachmentBit bits) {
					mClearBits = static_cast<uint8_t>(bits);
				}
				inline AttachmentBits operator|(AttachmentBit b) {
					return AttachmentBits(static_cast<uint8_t>(b));
				}
			};
		}

		namespace mesh {
			enum class Primitive : uint8_t {
				Points,
				Line,
				LineLoop,
				LineStrip,
				Triangles,
				TriangleStrip,
				TriangleFan,
			};

			enum class VertexType : uint8_t {
				Position,
				Normal,
				Texture0,
				Tangent,
				COUNT
			};
		}

		namespace texture {

			enum class Target : uint8_t {
				Texture1D,
				Texture2D,
				Texture3D,
				TextureCube
			};

			enum class Filters : uint8_t {
				Linear,
				Nearest,
				NearestMipmapNearest,
				LinearMipmapNearest,
				NearestMipmapLinear,
				LinearMipmapLinear
			};

			enum class Wraps : uint8_t {
				Repeat,
				Clamp,
				Mirrored
			};

			enum class InternalFormats : uint8_t {
				R8_UNORM,
				RG8_UNORM,
				RGB8_UNORM,
				RGBA8_UNORM,
				R16_UNORM,
				RG16_UNORM,
				RGB16_UNORM,
				RGBA16_UNORM,
				R16_UI,
				RG16_UI,
				RGB16_UI,
				RGBA16_UI,
				R16_F,
				RG16_F,
				RGB16_F,
				RGBA16_F,
				R32_F,
				RG32_F,
				RGB32_F,
				RGBA32_F,
				D16,
				D24,
				D32,
				D24S8
			};

			enum class BaseFormats : uint8_t {
				R,
				RG,
				RGB,
				RGBA,
				Depth,
				DepthStencil
			};
		}
	}

}