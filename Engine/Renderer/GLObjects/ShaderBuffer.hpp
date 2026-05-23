#pragma once

#include "Renderer/Types.hpp"

#include <stdint.h>
#include <optional>
#include <string>

namespace neo {

	class ShaderBuffer {
	public:
		ShaderBuffer(uint32_t byteSize, const uint8_t* data, const std::optional<std::string>& debugName);
		void update(uint32_t byteSize, const uint8_t* data, uint32_t offset = 0);

		template<typename T>
		void clear(uint32_t byteSize, T clearValue, uint32_t offset = 0);

		uint32_t mBufferID = 0;
		uint32_t mByteSize = 0;

		void destroy();
    private:
        void _clear(uint32_t byteSize, uint32_t offset, types::InternalFormats internalFormat, types::ByteFormats format, const uint8_t* clearValue);
	};

	template<typename T>
    void ShaderBuffer::clear(uint32_t byteSize, T clearValue, uint32_t offset) {
        static_assert(sizeof(T) == 4, "Clear value must be a 32-bit type (int, uint32_t, float, etc.)");

        // Deduce the OpenGL format based on whether T is signed, unsigned, or floating-point
		types::InternalFormats internalFormat;
        types::ByteFormats format;

        if constexpr (std::is_floating_point_v<T>) {
            internalFormat = types::InternalFormats::R32_F;
            format = types::ByteFormats::Float;
        }
        else if constexpr (std::is_signed_v<T>) {
            internalFormat = types::InternalFormats::R32_I;
            format = types::ByteFormats::Int;
        }
        else { // Unsigned
            internalFormat = types::InternalFormats::R32_UI;
            format = types::ByteFormats::UnsignedInt;
        }

        this->bind();
        glClearBufferSubData(
            GL_SHADER_STORAGE_BUFFER, // Assuming SSBO target, change if needed
            internalFormat,
            offset,
            sizeInBytes,
            format,
            type,
            &clearValue
        );

        _clear(byteSize, offset, internalFormat, format, reinterpret_cast<uint8_t*>(&clearValue));
    }
}
