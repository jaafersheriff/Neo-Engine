
#include "FrameCounter.hpp"

#include <imgui/imgui.h>
#include <implot/implot.h>
#include <microprofile.h>

#include <string>

namespace neo {
    namespace util {

        void FrameCounter::init(double runTime) {
            mLastFrameTime = runTime;
        }

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
                mFramesInCount = 0;
                mLastFPSTime = runTime;
            }

            if (mTimeStepList.size() == 1000) {
                mTimeStepList.erase(mTimeStepList.begin());
            }
            mTimeStepList.push_back(static_cast<float>(mTimeStep) * 1000.f);

        }

        void FrameCounter::imGuiEditor() const {
            // Translate FPS to floats
            std::vector<float> FPSfloats;
            std::vector<int> FPSInts = mFPSList;
            FPSfloats.resize(FPSInts.size());
            for (size_t i = 0; i < FPSInts.size(); i++) {
                FPSfloats[i] = static_cast<float>(FPSInts[i]);
            }
            if (ImPlot::BeginPlot("FPS")) {
                ImPlot::PlotLine("_", FPSfloats.data(), static_cast<int>(FPSfloats.size()));
                ImPlot::EndPlot();
            }
            if (ImPlot::BeginPlot("Frametime")) {
                ImPlot::PlotLine("__", mTimeStepList.data(), static_cast<int>(mTimeStepList.size()));
                ImPlot::EndPlot();
            }
            // ImGui::PlotLines("FPS", FPSfloats.data(), static_cast<int>(FPSfloats.size()), 0, std::to_string(mFPS).c_str());
            // ImGui::PlotLines("Frame time", mTimeStepList.data(), static_cast<int>(mTimeStepList.size()), 0, std::to_string(mTimeStep * 1000.f).c_str());
        }
    }
}