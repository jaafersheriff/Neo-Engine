#include "Jobs/JobHandle.hpp"

#include "Jobs/Internal/JobTask.hpp"

namespace neo {

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

		// Waiting here does NOT pick up other work
		mTask->mScheduler->WaitforTask(mTask->mCompletable, enki::TASK_PRIORITY_HIGH);
	}

	bool JobHandle::isComplete() const {
		return !mTask || mTask->mCompletable->GetIsComplete();
	}
}
