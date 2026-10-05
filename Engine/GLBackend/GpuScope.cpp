#include "GLBackend/pch.hpp"
#include "HAL/GpuScope.hpp"

#include "Util/Assert.hpp"

#include <GL/glew.h>
#include <tracy/TracyOpenGL.hpp>

#include <new>

namespace neo {

	static_assert(sizeof(tracy::GpuCtxScope) <= sizeof(bool) && alignof(tracy::GpuCtxScope) <= alignof(bool), "tracy::GpuCtxScope no longer fits in GpuScope::mProfilerScope");

	GpuScope::GpuScope(const tracy::SourceLocationData* srcloc, const char* name) {
		new (mProfilerScope) tracy::GpuCtxScope(srcloc, true);
#ifdef DEBUG_MODE
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(-1), name);
#else
		NEO_UNUSED(name);
#endif
	}

	GpuScope::~GpuScope() {
#ifdef DEBUG_MODE
		glPopDebugGroup();
#endif
		std::launder(reinterpret_cast<tracy::GpuCtxScope*>(mProfilerScope))->~GpuCtxScope();
	}
}
