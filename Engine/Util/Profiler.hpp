#pragma once

#include "Util/Assert.hpp"
#include "Util/HashedString.hpp"

#define _CAT(X,Y) _CAT2(X,Y)
#define _CAT2(X,Y) X##Y
#define TracyLine _CAT(__LINE__,U)
#include <tracy/Tracy.hpp>

#define TRACY_ZONEN(x) ZoneScopedNC(x, (neo::HashedString(x) & 0xfefefe) >> 1 )
#define TRACY_ZONE() TRACY_ZONEN(TracyFunction)


#include <memory>
#include <vector>
#include <array>

namespace neo {

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
			static constexpr int kMaxSamples = 400;

			// A fixed ring of samples
			struct Series {
				void mark(double time) {
					mSamples[mOffset] = static_cast<float>(time);
					mOffset = (mOffset + 1) % kMaxSamples;
					mCount = mCount < kMaxSamples ? mCount + 1 : kMaxSamples;
				}

				std::array<float, kMaxSamples> mSamples = {};
				int mCount = 0;
				int mOffset = 0;
			};

			int mRefreshRate = 60;

			uint64_t mFrame = 0;

			double mRunTime = 0.0;
			double mTimeStep = 0.0;

			double mBeginFrameTime = 0.0;

			// Main thread only
			Series mCPUFrametime; // Full CPU swap
			Series mNeoCPUTime; // Neo CPU tick

			// Written by the render thread - see the note on Series
			Series mNeoGPUTime;
		};
	}
}
