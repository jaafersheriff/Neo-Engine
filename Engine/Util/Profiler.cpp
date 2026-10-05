#include "Util/pch.hpp"

#include "Util/Log/Log.hpp"

#include "Profiler.hpp"

#include <ext/imgui_incl.hpp>
#include <implot.h>

void* operator new(std::size_t count) {
	auto ptr = malloc(count);
	TracyAlloc(ptr, count);
	return ptr;
}
void operator delete(void* ptr) noexcept {
	TracyFree(ptr);
	free(ptr);
}

namespace neo {
	namespace util {
		Profiler::Profiler(int refreshRate) 
			: mRefreshRate(refreshRate)
		{
		}

		Profiler::~Profiler() {
		}

		void Profiler::begin(double _runTime) {
			TRACY_ZONE();
			mFrame++;
			mBeginFrameTime = _runTime;
			mRunTime = _runTime;
		}

		void Profiler::markFrame(double _runTime) {
			double tickTime = (_runTime - mBeginFrameTime) * 1000.0;
			mNeoCPUTime.mark(tickTime);
		}

		void Profiler::markFrameGPU(double _runTime) {
			mNeoGPUTime.mark(_runTime);
		}

		void Profiler::end(double _runTime) {
			mTimeStep = (_runTime - mBeginFrameTime);
			mCPUFrametime.mark(mTimeStep * 1000.0); // Seconds to ms
		}

		void Profiler::imGuiEditor() const {
			TRACY_ZONE();

			ImGui::Begin("Profiler");
			char title[256];
			sprintf(title, "(FrameTime (%0.3fms)", mTimeStep * 1000.0);
			if (ImPlot::BeginPlot(title)) {
				ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_NoLabel);
				ImPlot::SetupAxis(ImAxis_Y1, "ms", ImPlotAxisFlags_NoInitialFit);
				ImPlot::SetupAxisLimits(ImAxis_X1, 0, kMaxSamples, ImPlotCond_Always);
				ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 2000.f / mRefreshRate, ImPlotCond_Always);

				ImPlot::SetNextLineStyle(ImVec4(0.5f, 1.0f, 0.0f, 1.0f));
				ImPlot::PlotLine("CPU", mCPUFrametime.mSamples.data(), mCPUFrametime.mCount, 1.0, 0.0, 0, mCPUFrametime.mOffset);

				ImPlot::SetNextLineStyle(ImVec4(0.11f, 0.63f, 0.2f, 1.0f));
				ImPlot::PlotLine("CPU tick", mNeoCPUTime.mSamples.data(), mNeoCPUTime.mCount, 1.0, 0.0, 0, mNeoCPUTime.mOffset);

				ImPlot::SetNextLineStyle(ImVec4(0.7f, 0.0f, 0.7f, 1.0f));
				ImPlot::PlotLine("GPU tick", mNeoGPUTime.mSamples.data(), mNeoGPUTime.mCount, 1.0, 0.0, 0, mNeoGPUTime.mOffset);

				ImPlot::EndPlot();
			}
			ImGui::End();
		}
	}
}
