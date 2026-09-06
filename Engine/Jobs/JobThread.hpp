#pragma once

#include <cstdint>

namespace neo {

	namespace detail {
		// Set once per thread when the scheduler starts it, and never written again.
		extern thread_local bool gIsRenderThread;
	}

	// True only on the job thread that owns the GL context.
	//
	// Deliberately a free function over a thread-local rather than a JobSystem method: the caller that
	// needs it most is the assert in ResourceManagerInterface::_resolveFinal, which runs tens of
	// thousands of times a frame, and NEO_ASSERT is live in RelWithDebInfo. Inlined this is a
	// thread-local load and a branch that always predicts; routed through the JobSystem it would be a
	// cross-TU call into the scheduler on every resolve.
	[[nodiscard]] inline bool isRenderThread() {
		return detail::gIsRenderThread;
	}

	// The threads work can be pinned to. Pinned is the enkiTS term for aiming work at a
	// specific thread - it is not CPU affinity.
	enum class JobThread : uint8_t {
		Main = 0,
		Render,
		COUNT
	};
}
