#pragma once

#include <functional>
#include <thread>
#include <optional>
#include <mutex>
#include <queue>

namespace neo {
	class RenderThread {
	public:
		using RenderFunc = std::function<void(void)>;

		// TODO - ability to disregard multithreading and run everything on main thread
		void start();
		void pushRenderFunc(RenderFunc func);
		void trigger();
		void wait();
		void kill();

		// TODO - this should be part of some threadmanager utility class
		bool isMainThread();
		bool isRenderThread();

	private:
		std::mutex mRenderQueueMutex;
		std::queue<RenderFunc> mRenderQueue;
		std::atomic<bool> mIsSleeping = false;

		std::thread mThread;
		std::condition_variable mWakeCondition;
		std::mutex mWakeMutex;

		std::thread::id mMainThreadID;
		std::thread::id mRenderThreadID;
	};
}
