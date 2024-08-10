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

		void start();
		void pushRenderFunc(RenderFunc func);
		void wait();
		void kill();

	private:
		std::mutex mRenderQueueMutex;
		std::queue<RenderFunc> mRenderQueue;

		std::thread mThread;
	};
}
