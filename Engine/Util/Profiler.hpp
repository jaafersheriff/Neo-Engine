#pragma once

#include "Util/Util.hpp"

#include <tracy/Tracy.hpp>
#include <GL/glew.h>
#include <tracy/TracyOpenGL.hpp>
#define TRACY_ZONEN(x) ZoneScopedNC(x, (neo::HashedString(x) & 0xfefefe) >> 1 )
#define TRACY_ZONE() TRACY_ZONEN(TracyFunction)


struct _NEO_GPU_CONTEXT {
	_NEO_GPU_CONTEXT(const char* name) {
#ifdef DEBUG_MODE
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(-1), name);
	}
#endif

	~_NEO_GPU_CONTEXT() {
#ifdef DEBUG_MODE
		if (glIsEnabled(GL_DEBUG_OUTPUT)) {
			glPopDebugGroup();
		}
#endif
	}
};
#define TRACY_GPUN(x) TRACY_ZONEN(x); TracyGpuZoneC(x, (neo::HashedString(x) & 0xfefefe) >> 1 ); _NEO_GPU_CONTEXT ___NEO_GPU_SCOPE##__LINE__(x)
#define TRACY_GPU() TRACY_GPUN(TracyFunction)

#include <memory>
#include <vector>

#ifndef NO_LOCAL_TRACY
namespace tracy {
	class View;
}
#endif

struct ImFont;

namespace neo {
	class RenderThread;

	namespace util {

		class Profiler {
		public:
			Profiler();
			~Profiler();
			Profiler(const Profiler&) = delete;
			Profiler& operator=(const Profiler&) = delete;

			void begin(double time);
			void end(double time);
			void imGuiEditor() const;

			uint64_t getFrameCount() const { return mFrame; }
			double getDeltaTime() const { return mTimeStep; } // I think this is in seconds

		private:
			double mLastFrameTime = 0.0;	/* Time at which last frame was rendered */
			uint64_t mFrame = 0;
			double mTimeStep = 0.0;		 /* Delta time */
		};
	}
}
