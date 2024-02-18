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
				// Wtf byte type are these?
				R8,
				RG8,
				RGB8,
				RGBA8,
				R16,
				RG16,
				RGB16,
				RGBA16,

				R16UI,
				RG16UI,
				RGB16UI,
				RGBA16UI,
				R16F,
				RG16F,
				RGB16F,
				RGBA16F,
				R32F,
				RG32F,
				RGB32F,
				RGBA32F,
				Depth16,
				Depth24,
				Depth32,
				Depth24Stencil8
			};

			enum class BaseFormats : uint8_t {
				Red,
				RG,
				RGB,
				RGBA,
				Depth,
				DepthStencil
			};
		}
	}

}