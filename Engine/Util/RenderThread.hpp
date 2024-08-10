#pragma once

#include <functional>
#include <thread>
#include <optional>
#include <mutex>

namespace neo {
	class RenderThread {
	public:
		using RenderFunc = std::function<void(void)>;

		void start();
		void setRenderFunc(RenderFunc func);
		void wait();
		void kill();

	private:
		std::mutex mRenderFuncMutex;
		std::optional<RenderFunc> mRenderFunc;

		std::thread mThread;
	};
}
