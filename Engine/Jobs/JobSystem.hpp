#pragma once

#include <cstdint>
#include <memory>

namespace neo {

	// The threads work can be aimed at. Main is enki's thread 0 - the thread that called init(). Every
	// other job thread is one this class creates and parks in a loop that runs only the tasks pinned to
	// it: it sleeps until something is pinned to it and never steals general work, which is what makes
	// it safe to hand one of them the GL context. Physics joins this list when Jolt lands.
	//
	// "Pinned" is enkiTS's term for work aimed at a specific thread. It is not CPU affinity - nothing
	// in Neo binds a thread to a core.
	enum class JobThread : uint8_t {
		Main = 0,
		Render,
		COUNT
	};

	enum class JobPriority : uint8_t {
		High,
		Normal,
		Low
	};

	// Neo's wrapper over enkiTS, in the same spirit as ECS over EnTT: no enki type appears in this
	// header, or in any header outside JobSystem.cpp. That keeps TaskScheduler.h out of every module's
	// include graph and leaves the thread topology free to change without touching call sites.
	//
	// This is the scheduler's bring-up only. Dispatching work - run/dispatch/parallelFor - lands next,
	// along with Engine ownership and the ImGui pane.
	class JobSystem {
	public:
		JobSystem();
		~JobSystem();
		JobSystem(const JobSystem&) = delete;
		JobSystem& operator=(const JobSystem&) = delete;

		// Must be called on the main thread: whichever thread calls this becomes thread 0, and only
		// threads the scheduler knows about may use its API.
		void init();
		// Waits for outstanding work, drops the job threads out of their loops, and joins. Safe to call
		// more than once, and called by the destructor.
		void shutdown();

		// Index of the calling thread, stable for the process lifetime and always < numThreads() for
		// any thread the scheduler knows about. This is what per-thread output buckets are indexed by,
		// which is how a wide loop accumulates without atomics.
		uint32_t threadIndex() const;
		// Where a named job thread ended up in that same index space.
		uint32_t threadIndexOf(JobThread thread) const;

		// numThreads() counts main, the compute workers and the job threads. numWorkers() counts only
		// the compute workers - the ones a task set can actually be scheduled onto - and is capped well
		// below the hardware thread count. See init().
		uint32_t numThreads() const;
		uint32_t numWorkers() const;

		// For long-lived loops on a job thread to poll, so they can fall out when shutdown starts.
		bool isShuttingDown() const;

	private:
		// enki::TaskScheduler is a member of Impl rather than of this class, because holding one here
		// would mean including TaskScheduler.h from this header - the exact thing the wrapper exists
		// to avoid.
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};
}
