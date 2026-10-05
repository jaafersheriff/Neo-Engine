#include "GLBackend/pch.hpp"
#include "HAL/GpuTimer.hpp"

#include "Util/Log/Log.hpp"

#include <GL/glew.h>

namespace neo {
	GpuTimer::Scope::Scope(uint32_t handle) {
		glBeginQuery(GL_TIME_ELAPSED, handle);
	}
	GpuTimer::Scope::~Scope() {
		glEndQuery(GL_TIME_ELAPSED);
	}

	void GpuTimer::init() {
		if (!_handlesValid()) {
			glGenQueries(2, mHandles.data());
		}
	}

	float GpuTimer::getGPUTime() const {
		if (!_handlesValid()) {
			return 0.f;
		}

		// Retrieve the inactive handle
		uint32_t handle = mUseHandle0 ? mHandles[1] : mHandles[0];

		int32_t done;
		glGetQueryObjectiv(handle, GL_QUERY_RESULT_AVAILABLE, &done);
		if (done) {
			uint64_t time;
			glGetQueryObjectui64v(handle, GL_QUERY_RESULT, &time);
			return time / 1000000.f;
		}

		NEO_LOG_W("GPU query not done?");
		return 0.f;
	}

	uint32_t GpuTimer::tickHandle() {
		mUseHandle0 = !mUseHandle0;
		return mUseHandle0 ? mHandles[0] : mHandles[1];
	}

	void GpuTimer::destroy() {
		if (!_handlesValid()) {
			return;
		}
		glDeleteQueries(2, mHandles.data());
		mHandles = { 0,0 };
	}

	bool GpuTimer::_handlesValid() const {
		return mHandles[0] && mHandles[1];
	}
}
