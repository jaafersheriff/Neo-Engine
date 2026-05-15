#pragma once

#include "Renderer/Types.hpp"

namespace neo {

	struct ShaderBarrier {
		ShaderBarrier(types::shader::Barrier barrierType);
		ShaderBarrier(const ShaderBarrier&) = delete;
		~ShaderBarrier();

	private:
		types::shader::Barrier mBarrierType;
	};
}
