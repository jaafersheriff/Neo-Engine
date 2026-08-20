#include "Renderer/pch.hpp"
#include "Renderer/RenderThread.hpp"

#include "Util/Profiler.hpp"
#include "Util/Util.hpp"

namespace neo {

	RenderThread::~RenderThread() {
		stop();
	}

	void RenderThread::start() {
		NEO_ASSERT(!mThread.joinable(), "Render thread is already running");
		{
			std::lock_guard<std::mutex> lock(mMutex);
			mKillSwitch = false;
		}
		mThread = std::thread(&RenderThread::_run, this);
	}

	void RenderThread::stop() {
		if (!mThread.joinable()) {
			return;
		}
		wait();
		{
			std::lock_guard<std::mutex> lock(mMutex);
			mKillSwitch = true;
		}
		mWorkReady.notify_one();
		mThread.join();
	}

	void RenderThread::dispatch(Job job, void* context) {
		NEO_ASSERT(mThread.joinable(), "Render thread is not running");
		{
			std::lock_guard<std::mutex> lock(mMutex);
			NEO_ASSERT(!mHasWork, "A job is already in flight - dispatch and wait must alternate");
			mJob = job;
			mContext = context;
			mHasWork = true;
		}
		mWorkReady.notify_one();
	}

	void RenderThread::wait() {
		if (!mThread.joinable()) {
			return;
		}
		TRACY_ZONEN("Wait for previous frame");
		std::unique_lock<std::mutex> lock(mMutex);
		mWorkDone.wait(lock, [this] { return !mHasWork; });
	}

	void RenderThread::runSync(Job job, void* context) {
		dispatch(job, context);
		wait();
	}

	void RenderThread::_run() {
		tracy::SetThreadName("Render Thread");

		while (true) {
			Job job = nullptr;
			void* context = nullptr;
			{
				// Zoned so the trace shows how much of the frame this thread spends asleep - that gap
				// is the headroom that letting the main thread run ahead would buy.
				TRACY_ZONEN("RenderThread::idle");
				std::unique_lock<std::mutex> lock(mMutex);
				mWorkReady.wait(lock, [this] { return mHasWork || mKillSwitch; });
				// Drain whatever is in flight before honouring the kill switch, so a stop() can never
				// abandon a frame the main thread is still waiting on.
				if (!mHasWork) {
					return;
				}
				job = mJob;
				context = mContext;
			}

			{
				TRACY_ZONEN("RenderThread::job");
				job(context);
			}

			{
				std::lock_guard<std::mutex> lock(mMutex);
				mHasWork = false;
				mJob = nullptr;
				mContext = nullptr;
			}
			mWorkDone.notify_one();
		}
	}
}
