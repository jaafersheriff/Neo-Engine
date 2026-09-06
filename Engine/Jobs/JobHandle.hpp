#pragma once

#include <cstdint>
#include <functional>
#include <memory>

namespace neo {

	// Matches ENKITS_TASK_PRIORITIES_NUM
	enum class JobPriority : uint8_t {
		High,
		Normal,
		Low
	};

	using JobFn = std::function<void()>;

	// An in-flight joinable job. Move-only, and the destructor blocks - a handle going out of scope is
	// a join, so a caller cannot accidentally outlive the task it dispatched.
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

		// Defined in Internal/JobTask.hpp, so that the enkiTS types it holds stay out of this header.
		// Every special member above is out of line for the same reason: the compiler needs Task to be
		// complete to destroy one, and it is not complete here.
		struct Task;
		explicit JobHandle(std::unique_ptr<Task> task);
		std::unique_ptr<Task> mTask;
	};
}
