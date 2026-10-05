#pragma once

#include "Util/Profiler.hpp"

namespace tracy {
	struct SourceLocationData;
}

namespace neo {

	// A named GPU zone: a profiler zone plus a debug group for capture tools.
	// Declared here so callers stay backend-agnostic; the backend library defines it.
	class GpuScope {
	public:
		GpuScope(const tracy::SourceLocationData* srcloc, const char* name);
		~GpuScope();
		GpuScope(const GpuScope&) = delete;
		GpuScope& operator=(const GpuScope&) = delete;

	private:
		// Holds the backend's profiler scope in place, so this header doesn't have to name its type
		alignas(bool) unsigned char mProfilerScope[sizeof(bool)];
	};
}

#ifdef TRACY_ENABLE
	#define TRACY_GPUN(x) TRACY_ZONEN(x); \
		static constexpr tracy::SourceLocationData _CAT(__neo_gpu_srcloc, __LINE__) { x, TracyFunction, TracyFile, (uint32_t)TracyLine, (neo::HashedString(x) & 0xfefefe) >> 1 }; \
		neo::GpuScope _CAT(__neo_gpu_scope, __LINE__)(&_CAT(__neo_gpu_srcloc, __LINE__), x)
#else
	#define TRACY_GPUN(x) neo::GpuScope _CAT(__neo_gpu_scope, __LINE__)(nullptr, x)
#endif
#define TRACY_GPU() TRACY_GPUN(TracyFunction)
