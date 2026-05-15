#pragma once

#include <stdint.h>
#include <optional>
#include <string>

namespace neo {

	class ShaderBuffer {
	public:
		ShaderBuffer(uint32_t byteSize, const uint8_t* data, const std::optional<std::string>& debugName);

		uint32_t mBufferID = 0;
		uint32_t mByteSize = 0;

		void destroy();
	};
}
