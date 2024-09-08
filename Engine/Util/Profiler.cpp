#include "Util/pch.hpp"

#include "Util/Util.hpp"

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

#define MAX_SAMPLES 400

namespace neo {
	namespace {
		inline void _markTime(double time, std::vector<float>& list, int& offset) {
			if (list.size() < MAX_SAMPLES) {
				list.emplace_back(static_cast<float>(time));
			}
			else {
				list[offset] = static_cast<float>(time);
				offset = (offset + 1) % MAX_SAMPLES;
			}
		}
	}
	namespace util {

		Profiler::Profiler(int refreshRate) 
			: mRefreshRate(refreshRate)
		{
			mCPUFrametime.reserve(MAX_SAMPLES);
			mGPUFrametime.reserve(MAX_SAMPLES);
			mFrametime.reserve(MAX_SAMPLES);
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
			_markTime(tickTime, mFrametime, mFrametimeOffset);
		}

		void Profiler::markFrameGPU(double _runTime) {
			_markTime(_runTime, mGPUFrametime, mGPUFrametimeOffset);
		}

		void Profiler::end(double _runTime) {
			mTimeStep = (_runTime - mBeginFrameTime);

			// Seconds to ms
			_markTime(mTimeStep * 1000.0, mCPUFrametime, mCPUFrametimeOffset);
		}

		void Profiler::imGuiEditor() const {
			ImGui::Begin("Profiler");
			if (ImPlot::BeginPlot(std::string("Frametime (" + std::to_string(mTimeStep * 1000.0) + "ms)").c_str())) {
				ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_NoLabel);
				ImPlot::SetupAxis(ImAxis_Y1, "ms", ImPlotAxisFlags_NoInitialFit);
				ImPlot::SetupAxisLimits(ImAxis_X1, 0, MAX_SAMPLES, ImPlotCond_Always);
				ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 2000.f / mRefreshRate, ImPlotCond_Always);

				ImPlot::SetNextLineStyle(ImVec4(0.5f, 1.0f, 0.0f, 1.0f));
				ImPlot::PlotLine("CPU", mCPUFrametime.data(), static_cast<int>(mCPUFrametime.size()), 1.0, 0.0, 0, mCPUFrametimeOffset);

				ImPlot::SetNextLineStyle(ImVec4(0.11f, 0.63f, 0.2f, 1.0f));
				ImPlot::PlotLine("tick", mFrametime.data(), static_cast<int>(mFrametime.size()), 1.0, 0.0, 0, mFrametimeOffset);

				ImPlot::SetNextLineStyle(ImVec4(0.7f, 0.0f, 0.7f, 1.0f));
				ImPlot::PlotLine("GPU", mGPUFrametime.data(), static_cast<int>(mGPUFrametime.size()), 1.0, 0.0, 0, mCPUFrametimeOffset);
				ImPlot::EndPlot();
			}
			ImGui::End();
		}
	}
}
