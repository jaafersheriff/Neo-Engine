#include "Util/pch.hpp"

#include "Util/Util.hpp"

#include "Profiler.hpp"

#ifndef NO_LOCAL_TRACY
#include <TracyView.hpp>
#endif


void* operator new(std::size_t count) {
    auto ptr = malloc(count);
    TracyAlloc(ptr, count);
    return ptr;
}
void operator delete(void* ptr) noexcept {
    TracyFree(ptr);
    free(ptr);
}

#ifndef NO_LOCAL_TRACY
namespace {
    void RunOnMainThread( std::function<void()> cb, bool forceDelay = false )
    {
        NEO_UNUSED(cb, forceDelay);
    }
    static void SetWindowTitleCallback( const char* title )
    {
        NEO_UNUSED(title);
    }
    
    static void AttentionCallback()
    {
    }
    static void SetupScaleCallback( float scale, ImFont*& cb_fixedWidth, ImFont*& cb_bigFont, ImFont*& cb_smallFont )
    {
        NEO_UNUSED(scale, cb_fixedWidth, cb_bigFont, cb_smallFont);
    }
}
#endif

namespace neo {
    namespace util {

        Profiler::Profiler(int refreshRate) {
#ifdef NO_LOCAL_TRACY
            NEO_UNUSED(refreshRate);
#else
            auto& io = ImGui::GetIO();
            mTracyServer = std::make_unique<tracy::View>( RunOnMainThread, "192.168.0.13", 8086, io.FontDefault, io.FontDefault, io.FontDefault, SetWindowTitleCallback, SetupScaleCallback, AttentionCallback);
            mTracyServer->GetViewData().frameTarget = refreshRate;
            mTracyServer->GetViewData().drawFrameTargets = true;
            mTracyServer->GetViewData().drawCpuUsageGraph = false;
#endif

            TracyPlotConfig("dt", tracy::PlotFormatType::Number, false, false, 0);
        }

        Profiler::~Profiler() {
#ifndef NO_LOCAL_TRACY
            mTracyServer.reset();
#endif
        }

        void Profiler::update(double _runTime) {
            TRACY_ZONE();
            /* Update delta time and FPS */
            float runTime = static_cast<float>(_runTime);
            mTotalFrames++;
            mTimeStep = runTime - mLastFrameTime;
            TracyPlot("dt", static_cast<float>(mTimeStep));
            mLastFrameTime = runTime;
        }

        void Profiler::imGuiEditor() const {
#ifndef NO_LOCAL_TRACY
            mTracyServer->Draw();
#endif
        }
    }
}