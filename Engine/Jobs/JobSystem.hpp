#pragma once

#include <cstdint>
#include <functional>
#include <memory>

namespace neo {

	namespace detail {
		// Set once per thread when the scheduler starts it, and never written again.
		extern thread_local bool gIsRenderThread;
	}

	// True only on the job thread that owns the GL context.
	//
	// Deliberately a free function over a thread-local rather than a JobSystem method: the caller that
	// needs it most is the assert in ResourceManagerInterface::_resolveFinal, which runs tens of
	// thousands of times a frame, and NEO_ASSERT is live in RelWithDebInfo. Inlined this is a
	// thread-local load and a branch that always predicts; routed through the JobSystem it would be a
	// cross-TU call into the scheduler on every resolve.
	[[nodiscard]] inline bool isRenderThread() {
		return detail::gIsRenderThread;
	}

	// The threads work can be aimed at. Main is enki's thread 0 - the thread that called init(). Every
	// other job thread is one this class creates and parks in a loop that runs only the tasks pinned to
	// it: it sleeps until something is pinned to it and never steals general work, which is what makes
	// it safe to hand one of them the GL context. Physics joins this list when Jolt lands.
	//
	// "Pinned" is enkiTS's term for work aimed at a specific thread. It is not CPU affinity - nothing
	// in Neo binds a thread to a core.
	//
	// Almost nothing should want a PinnedThread. It means "this must run where the GL context lives", so
	// it is the renderer and the things that touch GPU resources; everything else calls run/dispatch
	// and does not care where it lands.
	//
	// Note the ceiling that comes with that. A job running on the render thread can issue a
	// parallelFor and go wide like any other task - enki is braided, and the render thread helps run
	// the batches while it waits. What cannot go wide is GL itself: the context is bound to one thread
	// and commands issued from anywhere else are invalid, so draw submission is single-threaded no
	// matter how many workers are idle. That is an OpenGL constraint rather than a job system one, and
	// a modern API lifts it - per-thread command buffers recorded on workers and submitted from one
	// thread need nothing new here, only a different renderer.
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
		JobHandle();
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
