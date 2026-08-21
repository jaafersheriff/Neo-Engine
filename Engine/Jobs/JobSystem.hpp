#pragma once

#include <cstdint>
#include <memory>

namespace neo {

	// Pinned threads
	enum class JobThread : uint8_t {
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

		// Index of the calling thread
		uint32_t threadIndex() const;

		// Where a named job thread ended up 
		uint32_t threadIndexOf(JobThread thread) const;

		// Includes pinned threads
		uint32_t numThreads() const;
		// Excludes pinned threads
		uint32_t numWorkers() const;

		bool isShuttingDown() const;

	private:
		// Hide enkiTS types
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};
}
