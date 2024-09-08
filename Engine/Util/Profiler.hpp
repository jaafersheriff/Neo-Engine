#pragma once

#include "Util/Util.hpp"

#include <tracy/Tracy.hpp>
#include <GL/glew.h>
#include <tracy/TracyOpenGL.hpp>
#define TRACY_ZONEN(x) ZoneScopedNC(x, (neo::HashedString(x) & 0xfefefe) >> 1 )
#define TRACY_ZONE() TRACY_ZONEN(TracyFunction)


struct _NEO_GPU_SCOPE {
#ifdef DEBUG_MODE
	_NEO_GPU_SCOPE(const char* name) {
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(-1), name);
	}
	~_NEO_GPU_SCOPE() {
		glPopDebugGroup();
	}
#endif
};
#define TRACY_GPUN(x) TRACY_ZONEN(x); TracyGpuZoneC(x, (neo::HashedString(x) & 0xfefefe) >> 1 ); _NEO_GPU_SCOPE ___NEO_GPU_SCOPE##__LINE__(x)
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
			Profiler(int refreshRate);
			~Profiler();
			Profiler(const Profiler&) = delete;
			Profiler& operator=(const Profiler&) = delete;

			void begin(double time);
			void markFrame(double time);
			void markFrameGPU(double time);
			void end(double time);
			void imGuiEditor() const;

			uint64_t getFrameCount() const { return mFrame; }
			double getRunTime() const { return mRunTime; }
			double getDeltaTime() const { return mTimeStep; } // In seconds

		private:
			int mRefreshRate = 60;

			uint64_t mFrame = 0;

			double mRunTime = 0.0;
			double mTimeStep = 0.0;

			double mBeginFrameTime = 0.0;

			// Full swap
			std::vector<float> mCPUFrametime;
			int mCPUFrametimeOffset = 0;

			// GPU
			std::vector<float> mGPUFrametime;
			int mGPUFrametimeOffset = 0;

			// Neo tick
			std::vector<float> mFrametime;
			int mFrametimeOffset = 0;

		};
	}
}
