#pragma once

#include "Util/Util.hpp"

#include <tracy/Tracy.hpp>
#include <GL/glew.h>
#include <tracy/TracyOpenGL.hpp>
#define TRACY_ZONEN(x) ZoneScopedNC(x, (neo::HashedString(x) & 0xfefefe) >> 1 )
#define TRACY_ZONEF(fmt, ...) TRACY_ZONEN("") ; ZoneNameF(fmt, ##__VA_ARGS__) 
#define TRACY_ZONE() TRACY_ZONEN(TracyFunction)


struct _NEO_GPU_SCOPE {
	_NEO_GPU_SCOPE(const char* name) {
#ifdef DEBUG_MODE
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(-1), name);
#else
		NEO_UNUSED(name);
#endif
	}
	~_NEO_GPU_SCOPE() {
#ifdef DEBUG_MODE
		glPopDebugGroup();
#endif
	}
};
#define TRACY_GPUN(x) \
	TRACY_ZONEN(x); \
	TracyGpuZoneC(x, (neo::HashedString(x) & 0xfefefe) >> 1 ); \
	_NEO_GPU_SCOPE ___NEO_GPU_SCOPE##__LINE__(x)
#define TRACY_GPUF(x) \
	TRACY_ZONEF("%s", x); \
	TracyGpuZoneTransient(__tracy_gpu_zone, x, true); \
	_NEO_GPU_SCOPE ___NEO_GPU_SCOPE##__LINE__(x)
#define TRACY_GPU() TRACY_GPUN(TracyFunction)

#include <memory>
#include <vector>

namespace neo {

	namespace util {

		class Profiler {
		public:
			struct GPUQuery {

				void init();
				float getGPUTime() const;
				uint32_t tickHandle();
				void destroy();

				// Scoped or manual
				struct Scope {
					Scope(uint32_t handle);
					~Scope();
				};
				void start();
				void end();

			private:
				bool _handlesValid() const;

				std::array<uint32_t, 2> mHandles = { 0,0 };
				bool mUseHandle0 = true; // Double buffer
			};

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

			// Full CPU swap
			std::vector<float> mCPUFrametime;
			int mCPUFrametimeOffset = 0;

			// Neo CPU tick
			std::vector<float> mNeoCPUTime;
			int mNeoCPUTimeOffset = 0;

			// Neo GPU render
			std::vector<float> mNeoGPUTime;
			int mNeoGPUTimeOffset = 0;
		};
	}
}
