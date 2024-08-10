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
		void wait();
		void kill();

		// TODO - this should be part of some threadmanager utility class
		bool isMainThread();
		bool isRenderThread();

	private:
		std::mutex mRenderQueueMutex;
		std::queue<RenderFunc> mRenderQueue;

		std::thread mThread;

		std::thread::id mMainThreadID;
		std::thread::id mRenderThreadID;
	};
}
