#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

namespace neo {

	// One worker and a two-phase handshake: the main thread hands over a frame's work, then blocks
	// until the worker reports it finished. Deliberately not a job system - no queue, no pool, no
	// work stealing - because exactly one thing needs to run off the main thread, and the main thread
	// must never proceed while it is running.
	//
	// The job is a plain function pointer plus an opaque context rather than a std::function, so
	// dispatching a frame allocates nothing.
	class RenderThread {
	public:
		using Job = void(*)(void* context);

		RenderThread() = default;
		~RenderThread();
		RenderThread(const RenderThread&) = delete;
		RenderThread& operator=(const RenderThread&) = delete;

		void start();
		// Waits for any in-flight job, then joins. Safe to call more than once.
		void stop();

		// Hands work to the worker and returns immediately. Exactly one job may be in flight.
		void dispatch(Job job, void* context);

		// Blocks until the dispatched job has finished. Safe to call with nothing in flight, which is
		// what makes it usable as a plain quiesce point before tearing anything down.
		void wait();

		// dispatch + wait. For work that has to happen on this thread because the GL context lives
		// here, but that the caller cannot proceed without - engine start-up and demo swaps.
		void runSync(Job job, void* context);

	private:
		void _run();

		std::thread mThread;
		std::mutex mMutex;
		std::condition_variable mWorkReady;
		std::condition_variable mWorkDone;

		// All guarded by mMutex - they are condition variable predicates, so they have to be.
		Job mJob = nullptr;
		void* mContext = nullptr;
		bool mHasWork = false;
		bool mKillSwitch = false;
	};
}
