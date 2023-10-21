#pragma once

#define TRACY_ZONEN(x) ZoneScopedNC(x, (HashedString(x) & 0xfefefe) >> 1 )
#define TRACY_ZONE() TRACY_ZONEN(TracyFunction)
#define TRACY_GPUN(x) TRACY_ZONEN(x); TracyGpuZoneC(x, (HashedString(x) & 0xfefefe) >> 1 )
#define TRACY_GPU() TRACY_GPUN(TracyFunction)

#include <memory>

namespace tracy {
    class View;
}

namespace neo {
    namespace util {
        class Profiler {
        public:
            Profiler(int refreshRate);
            ~Profiler();
            Profiler(const Profiler&) = delete;
            Profiler& operator=(const Profiler&) = delete;

            void update(double);
            void imGuiEditor() const;

            double mTimeStep = 0.0;         /* Delta time */
            int mTotalFrames = 0;           /* Total frames since start up */
        private:
            std::unique_ptr<tracy::View> mTracyServer;
            double mLastFrameTime = 0.0;    /* Time at which last frame was rendered */
        };
    }
}
