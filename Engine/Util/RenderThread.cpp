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
					TRACY_ZONEN("Sleep");
					mIsSleeping.store(true);
					std::unique_lock<std::mutex> lock(mWakeMutex);
					mWakeCondition.wait(lock);
				}
				{
					TRACY_ZONEN("Execute");
					mIsSleeping.store(false);
					std::queue<RenderFunc> renderQueue;
					{
						std::lock_guard<std::mutex> lock(mRenderQueueMutex);
						std::swap(mRenderQueue, renderQueue);
					}
					while(!renderQueue.empty()) {
						renderQueue.front()();
						renderQueue.pop();
					}
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

	void RenderThread::trigger() {
#ifdef DEBUG_DISABLE_THREADING
		return;
#else
		mWakeCondition.notify_one();
		while (mIsSleeping.load()) {
			std::this_thread::yield(); // TODO - this is bad...?
		}
#endif
	}

	void RenderThread::wait() {
#ifdef DEBUG_DISABLE_THREADING
		return;
#else
		TRACY_ZONEN("Wait on render thread");
		bool queuedTasks = false;
		{
			std::lock_guard<std::mutex> lock(mRenderQueueMutex);
			queuedTasks = mRenderQueue.size() != 0;
		}

		while (!mIsSleeping.load() && queuedTasks) {
			std::this_thread::yield(); // TODO - this is bad...?

			std::lock_guard<std::mutex> lock(mRenderQueueMutex);
			queuedTasks = mRenderQueue.size() != 0;
		}
#endif
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
