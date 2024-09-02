#include "Util/pch.hpp"

#include "Util/Profiler.hpp"

#include "RenderThread.hpp"

//#define DEBUG_DISABLE_THREADING

namespace neo {

	void RenderThread::start() {
		mMainThreadID = std::this_thread::get_id();
		mThread = std::thread([this]() {
#ifndef DEBUG_DISABLE_THREADING
			tracy::SetThreadName("Render Thread");
			mRenderThreadID = std::this_thread::get_id();
			while (true) {
				{
					TRACY_ZONEN("Sleep...");
					std::this_thread::sleep_for(std::chrono::microseconds(100));
				}

				int jobs = 0;
				{
					std::lock_guard<std::mutex> lock(mRenderQueueMutex);
					jobs = static_cast<int>(mRenderQueue.size());
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
#endif
			});
		mThread.detach();
	}

	void RenderThread::pushRenderFunc(RenderFunc func) {
#ifdef DEBUG_DISABLE_THREADING
		func();
#else
		std::lock_guard<std::mutex> lock(mRenderQueueMutex);
		mRenderQueue.push(func);
#endif
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

			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}
	}

	void RenderThread::kill() {
		mThread.join();
	}

	bool RenderThread::isMainThread() {
#ifdef DEBUG_DISABLE_THREADING
		return true;
#else
		return std::this_thread::get_id() == mMainThreadID;
#endif
	}

	bool RenderThread::isRenderThread() {
#ifdef DEBUG_DISABLE_THREADING
		return true;
#else
		return std::this_thread::get_id() == mRenderThreadID;
#endif
	}
}
