#pragma once

#include "Renderer/Types.hpp"

#include <stdint.h>
#include <optional>
#include <string>

namespace neo {

	class ShaderBuffer {
	public:
		ShaderBuffer(types::buffer::Target target, uint32_t byteSize, const uint8_t* data, const std::optional<std::string>& debugName);
		void update(uint32_t byteSize, const uint8_t* data, uint32_t offset = 0);

		types::buffer::Target mTarget = types::buffer::Target::ShaderStorage;
		uint32_t mBufferID = 0;
		uint32_t mByteSize = 0;

		void destroy();
	};
}
