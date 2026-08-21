#pragma once

#include <cstdint>
#include <functional>
#include <memory>

namespace neo {

	// Pinned threads
	enum class PinnedThread : uint8_t {
		Main = 0,
		Render,
		COUNT
	};

	// Matches ENKITS_TASK_PRIORITIES_NUM
	enum class JobPriority : uint8_t {
		High,
		Normal,
		Low
	};

	using JobFn = std::function<void()>;
	using JobRangeFn = std::function<void(uint32_t begin, uint32_t end, uint32_t threadIndex)>;

	class JobHandle {
	public:
		JobHandle() = default;
		~JobHandle(); // Blocks for job to complete
		JobHandle(JobHandle&&) noexcept;
		JobHandle& operator=(JobHandle&&) noexcept;
		JobHandle(const JobHandle&) = delete;
		JobHandle& operator=(const JobHandle&) = delete;

		void wait();
		bool isComplete() const;
		bool isValid() const { return mTask != nullptr; }

	private:
		friend class JobSystem;

		struct Task;
		explicit JobHandle(std::unique_ptr<Task> task);
		std::unique_ptr<Task> mTask;
	};

	// enkiTS wrapper
	class JobSystem {
	public:
		JobSystem();
		~JobSystem();
		JobSystem(const JobSystem&) = delete;
		JobSystem& operator=(const JobSystem&) = delete;

		// Must be called on the main thread
		void init();

		void shutdown();

		// --- fire and forget: the JobSystem owns the task and destroys it on completion ---
		void run(JobFn fn, JobPriority priority = JobPriority::Normal);
		void waitForDetached(); // Blocks until everything run() started has finished
		uint32_t detachedCount() const;


		// --- joinable: only when the caller genuinely has to wait ---
		[[nodiscard]] JobHandle dispatch(JobFn fn, JobPriority priority = JobPriority::Normal);
		[[nodiscard]] JobHandle dispatchOn(PinnedThread thread, JobFn fn);
		void runSyncOn(PinnedThread thread, JobFn fn);

		// --- data parallel: blocking, returns once every batch has run ---
		// batchSize is a floor, not an exact chunk
		void parallelFor(uint32_t count, uint32_t batchSize, const JobRangeFn& fn, JobPriority priority = JobPriority::Normal);

		void pumpThisThread();

		// Index of the calling thread
		uint32_t threadIndex() const;

		// Where a named pinned thread ended up 
		uint32_t threadIndexOf(PinnedThread thread) const;

		// Includes pinned threads
		uint32_t numThreads() const;
		// Excludes pinned threads
		uint32_t numWorkers() const;

		bool isShuttingDown() const;

		void imguiEditor();

	private:
		// Hide enkiTS types
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};
}
