#pragma once

#define TRACY_ZONEN(x) ZoneScopedNC(x, (HashedString(x) & 0xfefefe) >> 1 )
#define TRACY_ZONE() TRACY_ZONEN(TracyFunction)
#define TRACY_GPUN(x) TRACY_ZONEN(x); TracyGpuZoneC(x, (HashedString(x) & 0xfefefe) >> 1 )
#define TRACY_GPU() TRACY_GPUN(TracyFunction)

#include <memory>
#include <imgui.h>

#ifndef NO_LOCAL_TRACY
namespace tracy {
    class View;
}
#endif

namespace neo {
    namespace util {
        class Profiler {
        public:
            Profiler(int refreshRate, float scale);
            ~Profiler();
            Profiler(const Profiler&) = delete;
            Profiler& operator=(const Profiler&) = delete;

            void update(double);
            void imGuiEditor(glm::uvec2 viewportSize, glm::uvec2 viewportPos, ImGuiID viewportID) const;

            double mTimeStep = 0.0;         /* Delta time */
            int mTotalFrames = 0;           /* Total frames since start up */
        private:
            double mLastFrameTime = 0.0;    /* Time at which last frame was rendered */
#ifndef NO_LOCAL_TRACY
            std::unique_ptr<tracy::View> mTracyServer;
#endif
        };
    }
}
