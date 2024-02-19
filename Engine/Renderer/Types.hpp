#pragma once

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