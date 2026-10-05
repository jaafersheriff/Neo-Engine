#pragma once

#include "HAL/Types.hpp"

namespace neo {

	struct ShaderBarrier {
		ShaderBarrier(types::shader::Barrier barrierType);
		ShaderBarrier(const ShaderBarrier&) = delete;
		~ShaderBarrier();

	private:
		types::shader::Barrier mBarrierType;
	};
}
