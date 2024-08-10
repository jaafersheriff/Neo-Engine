#include "Util/pch.hpp"

#include "RenderThread.hpp"

namespace neo {

	void RenderThread::start() {
		mThread = std::thread([this]() {
			tracy::SetThreadName("Render Thread");
			while (true) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				int jobs = 0;
				{
					std::lock_guard<std::mutex> lock(mRenderQueueMutex);
					jobs = mRenderQueue.size();
				}

				while (jobs > 0) {
					RenderFunc func;

					{
						std::lock_guard<std::mutex> lock(mRenderQueueMutex);
						func = mRenderQueue.front();
						mRenderQueue.pop();
					}

					func(); // Invoke
					jobs--;
				}
			}
			});
		mThread.detach();
	}

	void RenderThread::pushRenderFunc(RenderFunc func) {
		std::lock_guard<std::mutex> lock(mRenderQueueMutex);
		mRenderQueue.push(func);
	}

	void RenderThread::wait() {
		while (true) {
			{
				std::lock_guard<std::mutex> lock(mRenderQueueMutex);
				if (!mRenderQueue.empty()) {
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
