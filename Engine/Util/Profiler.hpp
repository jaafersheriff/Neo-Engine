#pragma once

#include <tracy/Tracy.hpp>
#include <GL/glew.h>
#include <tracy/TracyOpenGL.hpp>
#define TRACY_ZONEN(x) ZoneScopedNC(x, (neo::HashedString(x) & 0xfefefe) >> 1 )
#define TRACY_ZONE() TRACY_ZONEN(TracyFunction)
#define TRACY_GPUN(x) TRACY_ZONEN(x); TracyGpuZoneC(x, (neo::HashedString(x) & 0xfefefe) >> 1 )
#define TRACY_GPU() TRACY_GPUN(TracyFunction)

#include <memory>
#include <vector>

#ifndef NO_LOCAL_TRACY
namespace tracy {
	class View;
}
#endif

namespace neo {
	class RenderThread;

	namespace util {

		class Profiler {
		public:
			Profiler(int refreshRate, float scale, RenderThread& renderThread);
			~Profiler();
			Profiler(const Profiler&) = delete;
			Profiler& operator=(const Profiler&) = delete;

			void update(double);
			void imGuiEditor() const;

			double mTimeStep = 0.0;		 /* Delta time */
		private:
			double mLastFrameTime = 0.0;	/* Time at which last frame was rendered */
#ifndef NO_LOCAL_TRACY
			float mRefreshRate; // Milliseconds
			std::unique_ptr<tracy::View> mTracyServer;

			mutable std::vector<float> mCPUFrametime;
			mutable int mCPUFrametimeOffset;

			mutable std::vector<float> mGPUFrametime;
			mutable int mGPUFrametimeOffset;
#endif
		};
	}
}
