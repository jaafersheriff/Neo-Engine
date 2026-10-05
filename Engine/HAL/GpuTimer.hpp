#pragma once

#include <array>
#include <cstdint>

namespace neo {

	// Double-buffered GPU frame timer. Declared here so callers stay backend-agnostic;
	// the backend library defines it.
	class GpuTimer {
	public:
		void init();
		float getGPUTime() const;
		uint32_t tickHandle();
		void destroy();

		// Scoped or manual
		struct Scope {
			Scope(uint32_t handle);
			~Scope();
		};

	private:
		bool _handlesValid() const;

		std::array<uint32_t, 2> mHandles = { 0,0 };
		bool mUseHandle0 = true; // Double buffer
	};
}
