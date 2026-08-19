#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

namespace neo {

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

		// Blocks until the dispatched job has finished. Safe to call with nothing in flight
		void wait();

		// dispatch + wait
		void runSync(Job job, void* context);

	private:
		void _run();

		std::thread mThread;
		std::mutex mMutex;
		std::condition_variable mWorkReady;
		std::condition_variable mWorkDone;

		// All guarded by mMutex
		Job mJob = nullptr;
		void* mContext = nullptr;
		bool mHasWork = false;
		bool mKillSwitch = false;
	};
}
