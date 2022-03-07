
#include "FrameCounter.hpp"

#include <imgui/imgui.h>
#include <implot/implot.h>
#include <microprofile.h>

#include <string>

namespace neo {
    namespace util {

        void FrameCounter::update(double _runTime) {
            MICROPROFILE_SCOPEI("FrameCounter", "FrameCounter::update", MP_AUTO);
            /* Update delta time and FPS */
            float runTime = static_cast<float>(_runTime);
            mTotalFrames++;
            mTimeStep = runTime - mLastFrameTime;
            mLastFrameTime = runTime;
            mFramesInCount++;
            if (runTime - mLastFPSTime >= 1.0) {
                mFPS = mFramesInCount;
                if (mFPSList.size() == 100) {
                    mFPSList.erase(mFPSList.begin());
                }
                mFPSList.push_back(mFPS);
                mMaxFPS = std::max(mMaxFPS, mFPS);
                mFramesInCount = 0;
                mLastFPSTime = runTime;
            }
        }

        void FrameCounter::imGuiEditor() const {
            if (ImPlot::BeginPlot(std::string("FPS (" + std::to_string(mFPS) + ")").c_str())) {
                ImPlot::SetupAxis(ImAxis_X1, "Time (s)", ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoInitialFit  );
                ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoInitialFit | ImPlotAxisFlags_NoLabel );
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0, mMaxFPS + 10, ImPlotCond_Always);
                ImPlot::PlotLine("", mFPSList.data(), static_cast<int>(mFPSList.size()));
                ImPlot::EndPlot();
            }
        }
    }
}