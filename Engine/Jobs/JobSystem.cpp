#include "Jobs/JobSystem.hpp"

#include <ext/enki_incl.hpp>

#include "Util/Log/Log.hpp"

#include <algorithm>

namespace neo {

	namespace {
		// Main is JobThread::Main and already exists - it is whichever thread called init(). Every other
		// JobThread is one this class creates, so the count of threads to make is one less than COUNT.
		constexpr uint32_t kJobThreadCount = static_cast<uint32_t>(JobThread::COUNT) - 1;

		// Parks a job thread so that it only ever runs work pinned to it. WaitForNewPinnedTasks() sleeps
		// without running anything, so this thread never steals a task set - which is the whole reason
		// the render thread can live inside the scheduler at all. A stolen task must never be allowed to
		// sit in front of the frame.
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
		PinnedTaskLoop mJobThreadLoops[kJobThreadCount];

		// The job threads sit at the end of the index space, so worker indices stay contiguous from 1.
		uint32_t mFirstJobThread = 0;
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

		// Main and the render job thread both run hot every frame, so neither is counted as a compute
		// worker. The job threads are then added on top of that count rather than taken out of it: a
		// job thread is asleep unless work is pinned to it, so oversubscribing by them costs nothing,
		// where stealing a core from the compute pool would.
		//
		// The cap matters more than the subtraction. Uncapped, a 128-thread machine produced 126
		// workers, and waking that many for one batch of entities costs far more than it returns - the
		// hardware thread count is also logical, so it roughly doubles the real core count. 8 is a
		// starting number to be measured rather than a law; CP1c puts it in the ImGui pane.
		constexpr uint32_t kMaxWorkers = 8;
		constexpr uint32_t kReservedThreads = 2; // main + the render job thread
		const uint32_t hardwareThreads = enki::GetNumHardwareThreads();
		const uint32_t availableWorkers = hardwareThreads > kReservedThreads ? hardwareThreads - kReservedThreads : 1u;
		mImpl->mNumWorkers = std::min(availableWorkers, kMaxWorkers);

		enki::TaskSchedulerConfig config;
		config.numTaskThreadsToCreate = mImpl->mNumWorkers + kJobThreadCount;
		config.numExternalTaskThreads = 0;

		mImpl->mScheduler.Initialize(config);
		mImpl->mRunning = true;

		// GetNumTaskThreads() counts the initializing thread too, so the job threads are the tail of
		// [0, numThreads). Mirrors the layout in enkiTS's own WaitForNewPinnedTasks example.
		mImpl->mFirstJobThread = mImpl->mScheduler.GetNumTaskThreads() - kJobThreadCount;
		for (uint32_t i = 0; i < kJobThreadCount; ++i) {
			mImpl->mJobThreadLoops[i].mScheduler = &mImpl->mScheduler;
			mImpl->mJobThreadLoops[i].threadNum = mImpl->mFirstJobThread + i;
			mImpl->mScheduler.AddPinnedTask(&mImpl->mJobThreadLoops[i]);
		}

		NEO_LOG_I("JobSystem: %u hardware threads -> %u compute workers (cap %u), %u job threads",
			hardwareThreads, mImpl->mNumWorkers, kMaxWorkers, kJobThreadCount);
	}

	void JobSystem::shutdown() {
		if (!mImpl->mRunning) {
			return;
		}

		// Sets the shutdown flag that the job thread loops poll and wakes them out of
		// WaitForNewPinnedTasks, then waits for every task to finish before joining. Those loops have to
		// complete here: they are members of Impl, and ~ICompletable asserts on a task still in flight.
		mImpl->mScheduler.WaitforAllAndShutdown();
		mImpl->mRunning = false;
	}

	uint32_t JobSystem::threadIndex() const {
		return mImpl->mScheduler.GetThreadNum();
	}

	uint32_t JobSystem::threadIndexOf(JobThread thread) const {
		NEO_ASSERT(mImpl->mRunning, "JobSystem has no thread layout until init()");
		NEO_ASSERT(thread < JobThread::COUNT, "Invalid JobThread");
		if (thread == JobThread::Main) {
			return 0;
		}
		return mImpl->mFirstJobThread + static_cast<uint32_t>(thread) - 1;
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
