#include "Jobs/JobSystem.hpp"

#include <ext/enki_incl.hpp>
#include <ext/imgui_incl.hpp>

#include "Util/Log/Log.hpp"
#include "Util/Profiler.hpp"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

namespace neo {

	namespace {
		// Main doesn't count as a pinned thread..
		constexpr uint32_t kPinnedThreadCount = static_cast<uint32_t>(PinnedThread::COUNT) - 1;

		uint32_t sFirstPinnedThread = 0;

		// Null for anything that is not a named thread, so callers fall back to the worker naming
		const char* jobThreadName(PinnedThread thread) {
			switch (thread) {
				case PinnedThread::Main: return "Main";
				case PinnedThread::Render: return "Render";
				case PinnedThread::COUNT: break;
			}
			return nullptr;
		}

		void onThreadStart(uint32_t threadNum) {
			const char* named = nullptr;
			if (sFirstPinnedThread != 0 && threadNum >= sFirstPinnedThread) {
				named = jobThreadName(static_cast<PinnedThread>(threadNum - sFirstPinnedThread + 1));
			}

			char name[32] = {};
			if (named != nullptr) {
				snprintf(name, sizeof(name), "Neo %s", named);
			}
			else {
				snprintf(name, sizeof(name), "Neo Worker %u", threadNum);
			}
			tracy::SetThreadName(name);
		}

		enki::TaskPriority toEnkiPriority(JobPriority priority) {
			switch (priority) {
				case JobPriority::High: return enki::TASK_PRIORITY_HIGH;
				case JobPriority::Normal: return enki::TASK_PRIORITY_MED;
				case JobPriority::Low: return enki::TASK_PRIORITY_LOW;
			}
			return enki::TASK_PRIORITY_MED;
		}

		// Parks a job thread so that it only ever runs work pinned to it
		struct PinnedTaskLoop final : enki::IPinnedTask {
			enki::TaskScheduler* mScheduler = nullptr;

			void Execute() override {
				while (!mScheduler->GetIsShutdownRequested()) {
					mScheduler->WaitForNewPinnedTasks();
					mScheduler->RunPinnedTasks();
				}
			}
		};

		// Fire-and-forget metadata
		struct DetachedJobs {
			mutable std::mutex mMutex;
			std::condition_variable mDone;
			uint32_t mCount = 0;
			uint32_t mHighWater = 0;

			void begin() {
				std::lock_guard<std::mutex> lock(mMutex);
				++mCount;
				mHighWater = std::max(mHighWater, mCount);
			}

			void complete() {
				{
					std::lock_guard<std::mutex> lock(mMutex);
					NEO_ASSERT(mCount > 0, "A detached job completed without having been counted");
					--mCount;
				}
				mDone.notify_all();
			}

			void waitForAll() {
				std::unique_lock<std::mutex> lock(mMutex);
				mDone.wait(lock, [this] { return mCount == 0; });
			}

			uint32_t count() const {
				std::lock_guard<std::mutex> lock(mMutex);
				return mCount;
			}
		};

		struct DetachedDeleter final : enki::ICompletable {
			enki::Dependency mDependency;
			DetachedJobs* mOwner = nullptr;

			void OnDependenciesComplete(enki::TaskScheduler* scheduler, uint32_t threadNum) override {
				ICompletable::OnDependenciesComplete(scheduler, threadNum);
				// Deleting the job also deletes this, because this is a member of it - so read anything
				// still needed off it first.
				DetachedJobs* owner = mOwner;
				delete mDependency.GetDependencyTask();
				owner->complete();
			}
		};

		struct DetachedTaskSet final : enki::ITaskSet {
			JobFn mFn;
			DetachedDeleter mDeleter;

			DetachedTaskSet(DetachedJobs& owner, JobFn fn)
				: mFn(std::move(fn))
			{
				mDeleter.mOwner = &owner;
				mDeleter.SetDependency(mDeleter.mDependency, this);
			}

			void ExecuteRange(enki::TaskSetPartition, uint32_t) override {
				mFn();
			}
		};

	}

	// An in-flight joinable job
	struct JobHandle::Task {
		Task(enki::TaskScheduler& scheduler, JobFn fn, JobPriority priority)
			: mScheduler(&scheduler)
			, mTaskSet(1, [fn = std::move(fn)](enki::TaskSetPartition, uint32_t) { fn(); })
		{
			mTaskSet.m_Priority = toEnkiPriority(priority);
			mCompletable = &mTaskSet;
		}

		Task(enki::TaskScheduler& scheduler, uint32_t threadNum, JobFn fn)
			: mScheduler(&scheduler)
			, mPinned(threadNum, std::move(fn))
		{
			mCompletable = &mPinned;
		}

		enki::TaskScheduler* mScheduler = nullptr;
		enki::ICompletable* mCompletable = nullptr;
		enki::TaskSet mTaskSet;
		enki::LambdaPinnedTask mPinned;
	};

	JobHandle::JobHandle() = default;

	JobHandle::JobHandle(std::unique_ptr<Task> task)
		: mTask(std::move(task))
	{}

	JobHandle::JobHandle(JobHandle&&) noexcept = default;

	JobHandle& JobHandle::operator=(JobHandle&& other) noexcept {
		if (this != &other) {
			// Whatever is being replaced still has to be joined before its task can be destroyed.
			wait();
			mTask = std::move(other.mTask);
		}
		return *this;
	}

	JobHandle::~JobHandle() {
		wait();
	}

	void JobHandle::wait() {
		if (!mTask) {
			return;
		}
		mTask->mScheduler->WaitforTask(mTask->mCompletable);
	}

	bool JobHandle::isComplete() const {
		return !mTask || mTask->mCompletable->GetIsComplete();
	}


	struct JobSystem::Impl {
		enki::TaskScheduler mScheduler;
		PinnedTaskLoop mPinnedThreadLoops[kPinnedThreadCount];

		uint32_t mFirstPinnedThread = 0;
		uint32_t mNumWorkers = 0;
		bool mRunning = false;

		// Fire-and-forget bookkeeping
		DetachedJobs mDetached;

		// One counter per thread
		std::vector<uint32_t> mTestBatchesPerThread;
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

		sFirstPinnedThread = mImpl->mNumWorkers + 1;

		enki::TaskSchedulerConfig config;
		config.numTaskThreadsToCreate = mImpl->mNumWorkers + kPinnedThreadCount;
		config.numExternalTaskThreads = 0;
		config.profilerCallbacks.threadStart = onThreadStart;
		mImpl->mScheduler.Initialize(config);
		mImpl->mRunning = true;

		mImpl->mFirstPinnedThread = mImpl->mScheduler.GetNumTaskThreads() - kPinnedThreadCount;
		NEO_ASSERT(mImpl->mFirstPinnedThread == sFirstPinnedThread, "Thread naming and thread layout disagree");

		for (uint32_t i = 0; i < kPinnedThreadCount; ++i) {
			mImpl->mPinnedThreadLoops[i].mScheduler = &mImpl->mScheduler;
			mImpl->mPinnedThreadLoops[i].threadNum = mImpl->mFirstPinnedThread + i;
			mImpl->mScheduler.AddPinnedTask(&mImpl->mPinnedThreadLoops[i]);
		}

		mImpl->mTestBatchesPerThread.resize(mImpl->mScheduler.GetNumTaskThreads(), 0u);

		NEO_LOG_I("JobSystem: %u hardware threads -> %u compute workers (cap %u), %u job threads",
			hardwareThreads, mImpl->mNumWorkers, kMaxWorkers, kPinnedThreadCount);
	}

	void JobSystem::shutdown() {
		if (!mImpl->mRunning) {
			return;
		}

		// Wait for fire-and-forget tasks
		waitForDetached();

		mImpl->mScheduler.WaitforAllAndShutdown();
		mImpl->mRunning = false;
	}

	void JobSystem::run(JobFn fn, JobPriority priority) {
		NEO_ASSERT(mImpl->mRunning, "JobSystem is not initialized");

		DetachedTaskSet* job = new DetachedTaskSet(mImpl->mDetached, std::move(fn));
		job->m_Priority = toEnkiPriority(priority);
		// Counted before it is queued: a worker can finish it before this call returns.
		mImpl->mDetached.begin();
		mImpl->mScheduler.AddTaskSetToPipe(job);
	}

	JobHandle JobSystem::dispatch(JobFn fn, JobPriority priority) {
		NEO_ASSERT(mImpl->mRunning, "JobSystem is not initialized");

		auto task = std::make_unique<JobHandle::Task>(mImpl->mScheduler, std::move(fn), priority);
		mImpl->mScheduler.AddTaskSetToPipe(&task->mTaskSet);
		return JobHandle(std::move(task));
	}

	JobHandle JobSystem::dispatchOn(PinnedThread thread, JobFn fn) {
		NEO_ASSERT(mImpl->mRunning, "JobSystem is not initialized");

		auto task = std::make_unique<JobHandle::Task>(mImpl->mScheduler, threadIndexOf(thread), std::move(fn));
		mImpl->mScheduler.AddPinnedTask(&task->mPinned);
		return JobHandle(std::move(task));
	}

	void JobSystem::runSyncOn(PinnedThread thread, JobFn fn) {
		JobHandle handle = dispatchOn(thread, std::move(fn));
		handle.wait();
	}

	void JobSystem::parallelFor(uint32_t count, uint32_t batchSize, const JobRangeFn& fn, JobPriority priority) {
		NEO_ASSERT(mImpl->mRunning, "JobSystem is not initialized");
		if (count == 0) {
			return;
		}

		// The task set lives on this stack frame, which is safe because this function blocks 
		enki::TaskSet task(count, [&fn](enki::TaskSetPartition range, uint32_t threadNum) {
			fn(range.start, range.end, threadNum);
		});
		task.m_MinRange = std::max(1u, batchSize);
		task.m_Priority = toEnkiPriority(priority);

		mImpl->mScheduler.AddTaskSetToPipe(&task);
		mImpl->mScheduler.WaitforTask(&task);
	}

	void JobSystem::waitForDetached() {
		TRACY_ZONE();
		mImpl->mDetached.waitForAll();
	}

	uint32_t JobSystem::detachedCount() const {
		return mImpl->mDetached.count();
	}

	void JobSystem::pumpThisThread() {
		mImpl->mScheduler.RunPinnedTasks();
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

	void JobSystem::imguiEditor() {
		TRACY_ZONE();

		ImGui::Begin("Jobs");

		ImGui::Text("%u threads: 1 main, %u workers, %u job", numThreads(), numWorkers(), kPinnedThreadCount);
		for (uint8_t i = 0; i < static_cast<uint8_t>(PinnedThread::COUNT); ++i) {
			const PinnedThread thread = static_cast<PinnedThread>(i);
			ImGui::BulletText("%s: %u", jobThreadName(thread), threadIndexOf(thread));
		}
		ImGui::Text("This thread: %u", threadIndex());

		ImGui::Separator();
		ImGui::Text("Detached jobs in flight: %u (high water %u)", detachedCount(), mImpl->mDetached.mHighWater);
		if (ImGui::Button("Fire detached job (500ms)")) {
			run([] { std::this_thread::sleep_for(std::chrono::milliseconds(500)); }, JobPriority::Low);
		}

		ImGui::Separator();
		if (ImGui::Button("Fire test parallelFor")) {
			std::fill(mImpl->mTestBatchesPerThread.begin(), mImpl->mTestBatchesPerThread.end(), 0u);
			// One bump per batch, bucketed by thread - the same shape every wide loop will use, so this
			// exercises the per-thread accumulation as well as the split.
			parallelFor(1u << 20, 4096, [this](uint32_t, uint32_t, uint32_t threadIdx) {
				mImpl->mTestBatchesPerThread[threadIdx]++;
			});
		}
		uint32_t totalBatches = 0;
		uint32_t threadsUsed = 0;
		for (uint32_t batches : mImpl->mTestBatchesPerThread) {
			totalBatches += batches;
			threadsUsed += batches > 0 ? 1u : 0u;
		}
		ImGui::Text("Last test: %u batches across %u threads", totalBatches, threadsUsed);
		for (uint32_t i = 0; i < mImpl->mTestBatchesPerThread.size(); ++i) {
			if (mImpl->mTestBatchesPerThread[i] > 0) {
				ImGui::BulletText("thread %u: %u batches", i, mImpl->mTestBatchesPerThread[i]);
			}
		}

		ImGui::End();
	}
}
