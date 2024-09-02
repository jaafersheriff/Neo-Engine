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
					TRACY_ZONEN("Execute");
					std::queue<RenderFunc> renderQueue;
					{
						std::lock_guard<std::mutex> lock(mRenderQueueMutex);
						std::swap(mRenderQueue, renderQueue);
					}
					while(!renderQueue.empty()) {
						renderQueue.front()();
						renderQueue.pop();
						mFinishedJobIndex.fetch_add(1);
					}
				}
				{
					TRACY_ZONEN("Sleep");
					std::unique_lock<std::mutex> lock(mWakeMutex);
					mWakeCondition.wait(lock);
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
		mQueuedJobIndex++;
		std::lock_guard<std::mutex> lock(mRenderQueueMutex);
		mRenderQueue.push(func);
#endif
	}

	void RenderThread::wait() {
#ifdef DEBUG_DISABLE_THREADING
		return;
#else
		TRACY_ZONEN("Wait on render thread");
		while (mFinishedJobIndex.load() < mQueuedJobIndex) {
			mWakeCondition.notify_one();
			std::this_thread::yield();
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
