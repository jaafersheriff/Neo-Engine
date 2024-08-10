#include "Util/pch.hpp"

#include "RenderThread.hpp"

namespace neo {

	void RenderThread::start() {
		mThread = std::thread([this]() {
			tracy::SetThreadName("Render Thread");
			while (true) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				{
					std::lock_guard<std::mutex> lock(mRenderFuncMutex);
					if (mRenderFunc) {
						mRenderFunc.value()(); // Invoke
						mRenderFunc.reset();
					}
				}
			}
			});
		mThread.detach();
	}

	void RenderThread::setRenderFunc(RenderFunc func) {
		std::lock_guard<std::mutex> lock(mRenderFuncMutex);
		mRenderFunc = func;
	}

	void RenderThread::wait() {
		while (true) {
			{
				std::lock_guard<std::mutex> lock(mRenderFuncMutex);
				if (!mRenderFunc.has_value()) {
					return;
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	void RenderThread::kill() {
		mThread.join();
	}
}
