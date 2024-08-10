#include "Util/pch.hpp"

#include "Util/Profiler.hpp"

#include "RenderThread.hpp"

namespace neo {

	void RenderThread::start() {
		mMainThreadID = std::this_thread::get_id();
		mThread = std::thread([this]() {
			tracy::SetThreadName("Render Thread");
			mRenderThreadID = std::this_thread::get_id();
			while (true) {
				{
					TRACY_ZONEN("Sleep...");
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}

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
		TRACY_ZONEN("Wait on render thread");
		while (true) {
			{
				std::lock_guard<std::mutex> lock(mRenderQueueMutex);
				if (mRenderQueue.empty()) {
					return;
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	void RenderThread::kill() {
		mThread.join();
	}

	bool RenderThread::isMainThread() {
		return std::this_thread::get_id() == mMainThreadID;
	}

	bool RenderThread::isRenderThread() {
		return std::this_thread::get_id() == mRenderThreadID;
	}
}
