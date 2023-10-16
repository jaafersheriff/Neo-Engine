#include "Util/pch.hpp"

#include "Util/Util.hpp"

#include "Profiler.hpp"

#include <TracyView.hpp>


void* operator new(std::size_t count) {
    auto ptr = malloc(count);
    TracyAlloc(ptr, count);
    return ptr;
}
void operator delete(void* ptr) noexcept {
    TracyFree(ptr);
    free(ptr);
}

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

namespace neo {
    namespace util {

        Profiler::Profiler() {
            auto& io = ImGui::GetIO();
            view = std::make_unique<tracy::View>( RunOnMainThread, "192.168.0.13", 8086, io.FontDefault, io.FontDefault, io.FontDefault, SetWindowTitleCallback, SetupScaleCallback, AttentionCallback);

            // TracyPlotConfig("FPS", tracy::PlotFormatType::Number, false, false, 0);
            TracyPlotConfig("dt", tracy::PlotFormatType::Number, false, false, 0);


            mFPSList.name = HashedString("FPS");
        }

        Profiler::~Profiler() {
            view.reset();
        }

        void Profiler::update(double _runTime) {
            // ZoneScoped;
            /* Update delta time and FPS */
            float runTime = static_cast<float>(_runTime);
            mTotalFrames++;
            mTimeStep = runTime - mLastFrameTime;
            TracyPlot("dt", static_cast<float>(mTimeStep));
            mLastFrameTime = runTime;
            mFramesInCount++;
            if (runTime - mLastFPSTime >= 1.0) {
                mFPS = mFramesInCount;
                // if (mFPSList.size() == 100) {
                //     mFPSList.erase(mFPSList.begin());
                // }
                // mFPSList.push_back(mFPS);
                // TracyPlot("FPS", static_cast<int64_t>(mFPS));
                mMaxFPS = std::max(mMaxFPS, mFPS);
                mFramesInCount = 0;
                mLastFPSTime = runTime;
            }
        }

        void Profiler::imGuiEditor() {
            view->Draw();
            int t = 0;
            view->DrawPlot(mFPSList, 180.0, t, ImVec2(10, 10), false, 10, 200);
            
//             if (ImPlot::BeginPlot(std::string("FPS (" + std::to_string(mFPS) + ")").c_str())) {
//                 ImPlot::SetupAxis(ImAxis_X1, "Time (s)", ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoInitialFit  );
//                 ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoInitialFit | ImPlotAxisFlags_NoLabel );
//                 ImPlot::SetupAxisLimits(ImAxis_Y1, 0, mMaxFPS + 10, ImPlotCond_Always);
//                 ImPlot::PlotLine("", mFPSList.data(), static_cast<int>(mFPSList.size()));
//                 ImPlot::EndPlot();
//             }
        }
    }
}