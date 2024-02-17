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
				Nearest
			};

			enum class Wraps : uint8_t {
				Repeat,
				Clamp,
				Mirrored
			};

			enum class InternalFormats : uint8_t {
				RGBA8
			};

			enum class BaseFormats : uint8_t {
				RGBA
			};
		}
	}

}