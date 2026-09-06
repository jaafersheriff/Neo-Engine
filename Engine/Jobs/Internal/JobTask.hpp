#pragma once

// Internal to Engine/Jobs. Defines the enkiTS task that backs a JobHandle: JobHandle.cpp needs it to
// wait on and destroy one, JobSystem.cpp needs it to create one. Nothing outside Jobs/ should include
// this - it pulls enkiTS in, which is exactly the include the wrapper exists to keep out of the rest
// of the engine.

#include "Jobs/JobHandle.hpp"

#include <ext/enki_incl.hpp>

#include "Util/Profiler.hpp"

namespace neo {

	inline enki::TaskPriority toEnkiPriority(JobPriority priority) {
		switch (priority) {
			case JobPriority::High: return enki::TASK_PRIORITY_HIGH;
			case JobPriority::Normal: return enki::TASK_PRIORITY_MED;
			case JobPriority::Low: return enki::TASK_PRIORITY_LOW;
		}
		return enki::TASK_PRIORITY_MED;
	}

	// An in-flight joinable job. One of the two members is used depending on which constructor ran,
	// and mCompletable points at whichever it was - enki has no common task type to hold instead.
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
}
