#include "Jobs/JobSystem.hpp"

#include <ext/enki_incl.hpp>

#include "Util/Log/Log.hpp"

#include <algorithm>

namespace neo {

	namespace {
		// Main doesn't count as a pinned thread..
		constexpr uint32_t kPinnedThreadCount = static_cast<uint32_t>(PinnedThread::COUNT) - 1;

		struct PinnedTaskLoop final : enki::IPinnedTask {
			enki::TaskScheduler* mScheduler = nullptr;

			void Execute() override {
				while (!mScheduler->GetIsShutdownRequested()) {
					mScheduler->WaitForNewPinnedTasks();
					mScheduler->RunPinnedTasks();
				}
			}
		};
	}

	struct JobSystem::Impl {
		enki::TaskScheduler mScheduler;
		PinnedTaskLoop mPinnedThreadLoops[kPinnedThreadCount];

		uint32_t mFirstPinnedThread = 0;
		uint32_t mNumWorkers = 0;
		bool mRunning = false;
	};

	JobSystem::JobSystem()
		: mImpl(std::make_unique<Impl>())
	{}

	JobSystem::~JobSystem() {
		shutdown();
	}

	void JobSystem::init() {
		NEO_ASSERT(!mImpl->mRunning, "JobSystem is already initialized");

		constexpr uint32_t kMaxWorkers = 16;
		constexpr uint32_t kReservedThreads = 2; // main + the render job thread
		const uint32_t hardwareThreads = enki::GetNumHardwareThreads();
		const uint32_t availableWorkers = hardwareThreads > kReservedThreads ? hardwareThreads - kReservedThreads : 1u;
		mImpl->mNumWorkers = std::min(availableWorkers, kMaxWorkers);

		enki::TaskSchedulerConfig config;
		config.numTaskThreadsToCreate = mImpl->mNumWorkers + kPinnedThreadCount;
		config.numExternalTaskThreads = 0;

		mImpl->mScheduler.Initialize(config);
		mImpl->mRunning = true;

		mImpl->mFirstPinnedThread = mImpl->mScheduler.GetNumTaskThreads() - kPinnedThreadCount;
		for (uint32_t i = 0; i < kPinnedThreadCount; ++i) {
			mImpl->mPinnedThreadLoops[i].mScheduler = &mImpl->mScheduler;
			mImpl->mPinnedThreadLoops[i].threadNum = mImpl->mFirstPinnedThread + i;
			mImpl->mScheduler.AddPinnedTask(&mImpl->mPinnedThreadLoops[i]);
		}

		NEO_LOG_I("JobSystem: %u hardware threads -> %u compute workers (cap %u), %u job threads",
			hardwareThreads, mImpl->mNumWorkers, kMaxWorkers, kPinnedThreadCount);
	}

	void JobSystem::shutdown() {
		if (!mImpl->mRunning) {
			return;
		}

		mImpl->mScheduler.WaitforAllAndShutdown();
		mImpl->mRunning = false;
	}

	uint32_t JobSystem::threadIndex() const {
		return mImpl->mScheduler.GetThreadNum();
	}

	uint32_t JobSystem::threadIndexOf(PinnedThread thread) const {
		NEO_ASSERT(mImpl->mRunning, "JobSystem has no thread layout until init()");
		NEO_ASSERT(thread < PinnedThread::COUNT, "Invalid PinnedThread");
		if (thread == PinnedThread::Main) {
			return 0;
		}
		return mImpl->mFirstPinnedThread + static_cast<uint32_t>(thread) - 1;
	}

	uint32_t JobSystem::numThreads() const {
		return mImpl->mScheduler.GetNumTaskThreads();
	}

	uint32_t JobSystem::numWorkers() const {
		return mImpl->mNumWorkers;
	}

	bool JobSystem::isShuttingDown() const {
		return mImpl->mScheduler.GetIsShutdownRequested();
	}
}
