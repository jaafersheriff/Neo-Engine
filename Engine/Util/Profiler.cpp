#include "Util/pch.hpp"

#include "Util/Util.hpp"

#include "Profiler.hpp"

#include <imgui.h>

#ifndef NO_LOCAL_TRACY
#include <TracyView.hpp>
#include <TracyMouse.hpp>
#include <Fonts.hpp>
#include <implot.h>
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
	
	void RunOnMainThread( const std::function<void()>& cb, bool forceDelay = false )
	{
		cb();
	}
	
	static void AttentionCallback() {
		NEO_LOG_I("Tracy requires attention");
	}
}
#endif

#define MAX_SAMPLES 400

namespace neo {
	namespace util {

		Profiler::Profiler(int refreshRate, float scale) {
#ifdef NO_LOCAL_TRACY
			NEO_UNUSED(refreshRate, scale);

#else
			LoadFonts(scale, s_fixedWidth, s_smallFont, s_bigFont);

			tracy::Config config;
			config.threadedRendering = true;
			config.targetFps = refreshRate;
			mTracyServer = std::make_unique<tracy::View>(RunOnMainThread, "127.0.0.1", 8086, s_fixedWidth, s_smallFont, s_bigFont, nullptr, nullptr, AttentionCallback, config);
			mTracyServer->GetViewData().frameTarget = refreshRate;
			mTracyServer->GetViewData().drawFrameTargets = true;
			mTracyServer->GetViewData().drawCpuUsageGraph = false;

			mCPUFrametime.reserve(MAX_SAMPLES);
			mCPUFrametimeOffset = 0;
			mGPUFrametime.reserve(MAX_SAMPLES);
			mGPUFrametimeOffset = 0;

			mRefreshRate = 1000.f / refreshRate;

			// Let tracy catch up
			for (int i = 0; i < 4; i++) {
				TRACY_GPUN("Trash");
				std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(mRefreshRate * 2.f)));
				// TODO - bring back profiling TracyGpuCollect;
			}
#endif
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
			mTimeStep = runTime - mLastFrameTime;
			mLastFrameTime = runTime;

#ifndef NO_LOCAL_TRACY
			tracy::MouseFrame();
			{

				TRACY_ZONEN("Update frametime");
				float latestCPUTime = mTracyServer->GetFrametime();
				if (mCPUFrametime.size() < MAX_SAMPLES) {
					mCPUFrametime.emplace_back(latestCPUTime);
				}
				else {
					mCPUFrametime[mCPUFrametimeOffset] = latestCPUTime;
					mCPUFrametimeOffset = (mCPUFrametimeOffset + 1) % MAX_SAMPLES;
				}

				float latestGPUTime = mTracyServer->GetGPUFrametime();
				if (mGPUFrametime.size() < MAX_SAMPLES) {
					mGPUFrametime.emplace_back(latestGPUTime);
				}
				else {
					mGPUFrametime[mGPUFrametimeOffset] = latestGPUTime;
					mGPUFrametimeOffset = (mGPUFrametimeOffset + 1) % MAX_SAMPLES;
				}
			}
#endif
		}

		void Profiler::imGuiEditor() const {
#ifdef NO_LOCAL_TRACY
			return;
#else
			NEO_ASSERT(mTracyServer, "Tracy server doesn't exist..?");
			TRACY_ZONE();

			// Profiler is baked into the viewport dock space
			{
				TRACY_ZONEN("Draw Tracy");
				mTracyServer->Draw();
			}

			// Also have another simple graph for when the tracy profiler is collapsed
			{
				TRACY_ZONEN("Draw BasicProfiler");
				ImGui::Begin("BasicProfiler");
				if (ImPlot::BeginPlot(std::string("FPS (" + std::to_string(static_cast<int>(mTracyServer->GetFPS())) + ")").c_str())) {
					ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_NoLabel);
					ImPlot::SetupAxis(ImAxis_Y1, "ms", ImPlotAxisFlags_NoInitialFit);
					ImPlot::SetupAxisLimits(ImAxis_X1, 0, MAX_SAMPLES, ImPlotCond_Always);
					ImPlot::SetupAxisLimits(ImAxis_Y1, 0, mRefreshRate * 2.f, ImPlotCond_Always);

					ImPlot::SetNextLineStyle(ImVec4(0.5f, 1.0f, 0.0f, 1.0f));
					ImPlot::PlotLine("CPU", mCPUFrametime.data(), static_cast<int>(mCPUFrametime.size()), 1.0, 0.0, 0, mCPUFrametimeOffset);

					ImPlot::SetNextLineStyle(ImVec4(0.7, 0.0f, 0.7f, 1.0f));
					ImPlot::PlotLine("GPU", mGPUFrametime.data(), static_cast<int>(mGPUFrametime.size()), 1.0, 0.0, 0, mCPUFrametimeOffset);
					ImPlot::EndPlot();
				}
				ImGui::End();
			}
#endif // NO_LOCAL_TRACY

		}
	}
}