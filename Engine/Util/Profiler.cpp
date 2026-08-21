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

namespace neo {
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
			if (!_handlesValid()) {
				return;
			}
			glDeleteQueries(2, mHandles.data());
			mHandles = { 0,0 };
		}

		bool Profiler::GPUQuery::_handlesValid() const {
			return mHandles[0] && mHandles[1];
		}

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
