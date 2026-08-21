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
		// Main is JobThread::Main and already exists - it is whichever thread called init(). Every other
		// JobThread is one this class creates, so the count of threads to make is one less than COUNT.
		constexpr uint32_t kJobThreadCount = static_cast<uint32_t>(JobThread::COUNT) - 1;

		// Null for anything that is not a named job thread, so callers fall back to the worker naming
		// rather than inventing a placeholder.
		const char* jobThreadName(JobThread thread) {
			switch (thread) {
				case JobThread::Main: return "Main";
				case JobThread::Render: return "Render";
				case JobThread::COUNT: break;
			}
			return nullptr;
		}

		enki::TaskPriority toEnkiPriority(JobPriority priority) {
			switch (priority) {
				case JobPriority::High: return enki::TASK_PRIORITY_HIGH;
				case JobPriority::Normal: return enki::TASK_PRIORITY_MED;
				case JobPriority::Low: return enki::TASK_PRIORITY_LOW;
			}
			return enki::TASK_PRIORITY_MED;
		}

		// enki's profiler callbacks are plain function pointers with no user data, so the layout they
		// need has to live out here. There is one JobSystem per process, and this is written before
		// Initialize() starts any thread that reads it.
		uint32_t sFirstJobThread = 0;

		void onThreadStart(uint32_t threadNum) {
			const char* named = nullptr;
			if (sFirstJobThread != 0 && threadNum >= sFirstJobThread) {
				named = jobThreadName(static_cast<JobThread>(threadNum - sFirstJobThread + 1));
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

		// The whole of what fire-and-forget needs to be waitable: a count, and a way to block until it
		// reaches zero. run() increments before the job is queued and the completion action decrements.
		// The demo-swap gate will ask this instead of counting entities.
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

		// enkiTS's completion-action pattern (example/CompletionAction.cpp): once the job is finished the
		// scheduler is done dereferencing it, so the action can delete it. This is what lets run() hand
		// back nothing at all - with no handle there is nothing to dangle, and so no need for slot pools
		// or generation counters to make one safe.
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
				TRACY_ZONEN("Job (detached)");
				mFn();
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

		// Fire-and-forget bookkeeping. Deliberately its own type rather than loose members: the
		// completion action that deletes a finished job has to reach it, and it should not need access
		// to the whole of Impl to do that.
		DetachedJobs mDetached;

		// One counter per thread, for the pane's test button. Per-thread buckets are the pattern every
		// wide loop will use to accumulate without atomics, so the test exercises it too.
		std::vector<uint32_t> mTestBatchesPerThread;
	};

	// One in-flight joinable job. Both task objects are always constructed and only one is ever used -
	// a never-run enki task is inert and its destructor is happy, which is a fair trade against carrying
	// a variant around for something created a handful of times a frame.
	struct JobHandle::Task {
		Task(enki::TaskScheduler& scheduler, JobFn fn, JobPriority priority)
			: mScheduler(&scheduler)
			, mTaskSet(1, [fn = std::move(fn)](enki::TaskSetPartition, uint32_t) {
				TRACY_ZONEN("Job");
				fn();
			})
		{
			mTaskSet.m_Priority = toEnkiPriority(priority);
			mCompletable = &mTaskSet;
		}

		Task(enki::TaskScheduler& scheduler, uint32_t threadNum, JobFn fn)
			: mScheduler(&scheduler)
			, mPinned(threadNum, [fn = std::move(fn)] {
				TRACY_ZONEN("Job (pinned)");
				fn();
			})
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

		// Waiting here does NOT pick up other work, and that is the point. This is what the frame
		// handshake blocks on, so it has a deadline; a task is not preemptible once started, and a glTF
		// parse runs for seconds. Letting main pull one in would stall the frame loop outright. Today a
		// worker almost always claims a job first, so it has never actually happened - which is luck,
		// not a design.
		//
		// Passing the highest priority as the floor is how enki expresses "run nothing else": no engine
		// work is ever enqueued at JobPriority::High. Tasks pinned to the waiting thread still run,
		// since enki gives those the high priority by default - that is wanted, and it is the same work
		// pumpThisThread would do.
		//
		// parallelFor deliberately does not use this path. See the wait at the bottom of it.
		mTask->mScheduler->WaitforTask(mTask->mCompletable, enki::TASK_PRIORITY_HIGH);
	}

	bool JobHandle::isComplete() const {
		return !mTask || mTask->mCompletable->GetIsComplete();
	}

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
		// starting number to be measured rather than a law.
		constexpr uint32_t kMaxWorkers = 8;
		constexpr uint32_t kReservedThreads = 2; // main + the render job thread
		const uint32_t hardwareThreads = enki::GetNumHardwareThreads();
		const uint32_t availableWorkers = hardwareThreads > kReservedThreads ? hardwareThreads - kReservedThreads : 1u;
		mImpl->mNumWorkers = std::min(availableWorkers, kMaxWorkers);

		// Written before Initialize() because that is what starts the threads whose start callback reads
		// it. GetNumTaskThreads() would give the same answer, but only after the threads already exist.
		sFirstJobThread = mImpl->mNumWorkers + 1;

		enki::TaskSchedulerConfig config;
		config.numTaskThreadsToCreate = mImpl->mNumWorkers + kJobThreadCount;
		config.numExternalTaskThreads = 0;
		config.profilerCallbacks.threadStart = onThreadStart;

		mImpl->mScheduler.Initialize(config);
		mImpl->mRunning = true;

		// GetNumTaskThreads() counts the initializing thread too, so the job threads are the tail of
		// [0, numThreads). Mirrors the layout in enkiTS's own WaitForNewPinnedTasks example.
		mImpl->mFirstJobThread = mImpl->mScheduler.GetNumTaskThreads() - kJobThreadCount;
		NEO_ASSERT(mImpl->mFirstJobThread == sFirstJobThread, "Thread naming and thread layout disagree");

		for (uint32_t i = 0; i < kJobThreadCount; ++i) {
			mImpl->mJobThreadLoops[i].mScheduler = &mImpl->mScheduler;
			mImpl->mJobThreadLoops[i].threadNum = mImpl->mFirstJobThread + i;
			mImpl->mScheduler.AddPinnedTask(&mImpl->mJobThreadLoops[i]);
		}

		mImpl->mTestBatchesPerThread.resize(mImpl->mScheduler.GetNumTaskThreads(), 0u);

		NEO_LOG_I("JobSystem: %u hardware threads -> %u compute workers (cap %u), %u job threads",
			hardwareThreads, mImpl->mNumWorkers, kMaxWorkers, kJobThreadCount);
	}

	void JobSystem::shutdown() {
		if (!mImpl->mRunning) {
			return;
		}

		// Anything fire-and-forget still in flight owns heap memory and holds a pointer back into Impl,
		// so it has to land before the scheduler goes away.
		waitForDetached();

		// Sets the shutdown flag that the job thread loops poll and wakes them out of
		// WaitForNewPinnedTasks, then waits for every task to finish before joining. Those loops have to
		// complete here: they are members of Impl, and ~ICompletable asserts on a task still in flight.
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

	JobHandle JobSystem::dispatchOn(JobThread thread, JobFn fn) {
		NEO_ASSERT(mImpl->mRunning, "JobSystem is not initialized");

		auto task = std::make_unique<JobHandle::Task>(mImpl->mScheduler, threadIndexOf(thread), std::move(fn));
		mImpl->mScheduler.AddPinnedTask(&task->mPinned);
		return JobHandle(std::move(task));
	}

	void JobSystem::runSyncOn(JobThread thread, JobFn fn) {
		JobHandle handle = dispatchOn(thread, std::move(fn));
		handle.wait();
	}

	void JobSystem::parallelFor(uint32_t count, uint32_t batchSize, const JobRangeFn& fn, JobPriority priority) {
		NEO_ASSERT(mImpl->mRunning, "JobSystem is not initialized");
		if (count == 0) {
			return;
		}

		// The task set lives on this stack frame, which is safe precisely because this call blocks until
		// every batch of it has run.
		enki::TaskSet task(count, [&fn](enki::TaskSetPartition range, uint32_t threadNum) {
			// One zone per batch, which is also the cheapest way to see how enki actually split the set.
			TRACY_ZONEN("parallelFor");
			fn(range.start, range.end, threadNum);
		});
		task.m_MinRange = std::max(1u, batchSize);
		task.m_Priority = toEnkiPriority(priority);

		mImpl->mScheduler.AddTaskSetToPipe(&task);
		// The opposite policy to JobHandle::wait, on purpose. The caller is blocked on exactly this task
		// set, so running its batches here is not the waiter taking on unrelated work - it is the waiter
		// doing the work it is waiting for, which strictly lowers the time to finish.
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

	void JobSystem::imguiEditor() {
		TRACY_ZONE();

		ImGui::Begin("Jobs");

		ImGui::Text("%u threads: 1 main, %u workers, %u job", numThreads(), numWorkers(), kJobThreadCount);
		for (uint8_t i = 0; i < static_cast<uint8_t>(JobThread::COUNT); ++i) {
			const JobThread thread = static_cast<JobThread>(i);
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
