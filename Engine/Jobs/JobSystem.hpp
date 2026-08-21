#pragma once

#include <cstdint>
#include <functional>
#include <memory>

namespace neo {

	// The threads work can be aimed at. Main is enki's thread 0 - the thread that called init(). Every
	// other job thread is one this class creates and parks in a loop that runs only the tasks pinned to
	// it: it sleeps until something is pinned to it and never steals general work, which is what makes
	// it safe to hand one of them the GL context. Physics joins this list when Jolt lands.
	//
	// "Pinned" is enkiTS's term for work aimed at a specific thread. It is not CPU affinity - nothing
	// in Neo binds a thread to a core.
	//
	// Almost nothing should want a JobThread. It means "this must run where the GL context lives", so
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
	enum class JobThread : uint8_t {
		Main = 0,
		Render,
		COUNT
	};

	// Low is for work that should fill idle cores without ever pushing the frame around - asset loads,
	// shader mod-time sweeps. High is deliberately left unused by engine work: waiting at High is then
	// a wait that runs nothing else, which is the escape hatch if helping-out-while-waiting ever bites.
	enum class JobPriority : uint8_t {
		High,
		Normal,
		Low
	};

	using JobFn = std::function<void()>;
	using JobRangeFn = std::function<void(uint32_t begin, uint32_t end, uint32_t threadIndex)>;

	// Move-only owner of one in-flight job, for callers that need a join point. The destructor waits,
	// so a handle can never drop a task the scheduler still holds a pointer to.
	//
	// Work that nobody needs to join does not need one of these at all - see JobSystem::run.
	class JobHandle {
	public:
		// Every special member is out of line, including this one: Task is incomplete here, so anything
		// the compiler defines inline would have to instantiate unique_ptr's deleter against it - which
		// is an error even when the path is never taken.
		JobHandle();
		~JobHandle();
		JobHandle(JobHandle&&) noexcept;
		JobHandle& operator=(JobHandle&&) noexcept;
		JobHandle(const JobHandle&) = delete;
		JobHandle& operator=(const JobHandle&) = delete;

		// Blocks until the job is done, running other tasks while it waits. Call from main or from
		// inside another job - never from a thread the scheduler does not know about.
		void wait();
		bool isComplete() const;
		bool isValid() const { return mTask != nullptr; }

	private:
		friend class JobSystem;
		struct Task;
		explicit JobHandle(std::unique_ptr<Task> task);

		std::unique_ptr<Task> mTask;
	};

	// Neo's wrapper over enkiTS, in the same spirit as ECS over EnTT: no enki type appears in this
	// header, or in any header outside JobSystem.cpp. That keeps TaskScheduler.h out of every module's
	// include graph and leaves the thread topology free to change without touching call sites.
	class JobSystem {
	public:
		JobSystem();
		~JobSystem();
		JobSystem(const JobSystem&) = delete;
		JobSystem& operator=(const JobSystem&) = delete;

		// Must be called on the main thread: whichever thread calls this becomes thread 0, and only
		// threads the scheduler knows about may use its API.
		void init();
		// Waits for outstanding work, drops the job threads out of their loops, and joins. Safe to call
		// more than once, and called by the destructor.
		void shutdown();

		// --- fire and forget: the JobSystem owns the task and destroys it on completion ---
		void run(JobFn fn, JobPriority priority = JobPriority::Normal);

		// --- joinable: only when the caller genuinely has to wait ---
		[[nodiscard]] JobHandle dispatch(JobFn fn, JobPriority priority = JobPriority::Normal);
		[[nodiscard]] JobHandle dispatchOn(JobThread thread, JobFn fn);
		void runSyncOn(JobThread thread, JobFn fn);

		// --- data parallel: blocking, returns once every batch has run ---
		// batchSize is a floor, not an exact chunk: enki runs ranges of max(count / partitions,
		// batchSize), so this stops a set being cut up smaller than it is worth scheduling.
		void parallelFor(uint32_t count, uint32_t batchSize, const JobRangeFn& fn, JobPriority priority = JobPriority::Normal);

		// --- quiesce: demo swap and shutdown ---
		// Blocks until everything run() started has finished. Call from main.
		void waitForDetached();
		uint32_t detachedCount() const;

		// Runs whatever is pinned to the calling thread. Main calls this once a frame, which is what
		// makes JobThread::Main a destination a worker can hand work back to.
		void pumpThisThread();

		// Index of the calling thread, stable for the process lifetime and always < numThreads() for
		// any thread the scheduler knows about. This is what per-thread output buckets are indexed by,
		// which is how a wide loop accumulates without atomics.
		uint32_t threadIndex() const;
		// Where a named job thread ended up in that same index space.
		uint32_t threadIndexOf(JobThread thread) const;

		// numThreads() counts main, the compute workers and the job threads. numWorkers() counts only
		// the compute workers - the ones a task set can actually be scheduled onto - and is capped well
		// below the hardware thread count. See init().
		uint32_t numThreads() const;
		uint32_t numWorkers() const;

		// For long-lived loops on a job thread to poll, so they can fall out when shutdown starts.
		bool isShuttingDown() const;

		void imguiEditor();

	private:
		// enki::TaskScheduler is a member of Impl rather than of this class, because holding one here
		// would mean including TaskScheduler.h from this header - the exact thing the wrapper exists
		// to avoid.
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};
}
