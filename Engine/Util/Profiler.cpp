#include "Util/pch.hpp"

#include "Util/Util.hpp"

#include "Profiler.hpp"

#include <ext/imgui_incl.hpp>
#include <implot.h>

#pragma optimize("", off)

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
		Profiler::GPUQuery::Scope::Scope(uint32_t handle) {
			glBeginQuery(GL_TIME_ELAPSED, handle);
		}
		Profiler::GPUQuery::Scope::~Scope() {
			glEndQuery(GL_TIME_ELAPSED);
		}

		void Profiler::GPUQuery::init() {
			if (!_handlesValid()) {
				glGenQueries(2, mHandles.data());
			}
		}

		float Profiler::GPUQuery::getGPUTime() const {
			if (!_handlesValid()) {
				return 0.f;
			}

			// Retrieve the inactive handle
			uint32_t handle = mUseHandle0 ? mHandles[1] : mHandles[0];

			int32_t done;
			glGetQueryObjectiv(handle, GL_QUERY_RESULT_AVAILABLE, &done);
			if (done) {
				uint64_t time;
				glGetQueryObjectui64v(handle, GL_QUERY_RESULT, &time);
				return time / 1000000.f;
			}

			NEO_LOG_W("GPU query not done?");
			return 0.f;
		}

		uint32_t Profiler::GPUQuery::tickHandle() {
			mUseHandle0 = !mUseHandle0;
			return mUseHandle0 ? mHandles[0] : mHandles[1];
		}

		void Profiler::GPUQuery::destroy() {
			glDeleteQueries(2, mHandles.data());
			mHandles = { 0,0 };
		}

		bool Profiler::GPUQuery::_handlesValid() const {
			return mHandles[0] && mHandles[1];
		}

		Profiler::Profiler(int refreshRate) 
			: mRefreshRate(refreshRate)
		{
			mCPUFrametime.reserve(MAX_SAMPLES);
			mNeoCPUTime.reserve(MAX_SAMPLES);
			mNeoGPUTime.reserve(MAX_SAMPLES);
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
			_markTime(tickTime, mNeoCPUTime, mNeoCPUTimeOffset);
		}

		void Profiler::markFrameGPU(double _runTime) {
			_markTime(_runTime, mNeoGPUTime, mNeoGPUTimeOffset);
		}

		void Profiler::end(double _runTime) {
			mTimeStep = (_runTime - mBeginFrameTime);
			_markTime(mTimeStep * 1000.0, mCPUFrametime, mCPUFrametimeOffset); // Seconds to ms
		}

		void Profiler::imGuiEditor() const {
			ImGui::Begin("Profiler");
			char title[256];
			sprintf(title, "(FrameTime (%0.3fms)", mTimeStep * 1000.0);
			if (ImPlot::BeginPlot(title)) {
				ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_NoLabel);
				ImPlot::SetupAxis(ImAxis_Y1, "ms", ImPlotAxisFlags_NoInitialFit);
				ImPlot::SetupAxisLimits(ImAxis_X1, 0, MAX_SAMPLES, ImPlotCond_Always);
				ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 2000.f / mRefreshRate, ImPlotCond_Always);

				ImPlot::SetNextLineStyle(ImVec4(0.5f, 1.0f, 0.0f, 1.0f));
				ImPlot::PlotLine("CPU", mCPUFrametime.data(), static_cast<int>(mCPUFrametime.size()), 1.0, 0.0, 0, mCPUFrametimeOffset);

				ImPlot::SetNextLineStyle(ImVec4(0.11f, 0.63f, 0.2f, 1.0f));
				ImPlot::PlotLine("CPU tick", mNeoCPUTime.data(), static_cast<int>(mNeoCPUTime.size()), 1.0, 0.0, 0, mNeoCPUTimeOffset);

				ImPlot::SetNextLineStyle(ImVec4(0.7f, 0.0f, 0.7f, 1.0f));
				ImPlot::PlotLine("GPU tick", mNeoGPUTime.data(), static_cast<int>(mNeoGPUTime.size()), 1.0, 0.0, 0, mNeoGPUTimeOffset);

				ImPlot::EndPlot();
			}
			ImGui::End();
		}
	}
}
