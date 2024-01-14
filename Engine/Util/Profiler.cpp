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
void* zigzagTex;
namespace {
    
    void RunOnMainThread( const std::function<void()>& cb, bool forceDelay = false )
    {
        cb();
    }
    
    static void AttentionCallback() {
        NEO_LOG_E("Tracy requires attention");
    }
}
#endif

namespace neo {
    namespace util {

        Profiler::Profiler(int refreshRate, float scale) {
#ifdef NO_LOCAL_TRACY
            NEO_UNUSED(refreshRate);

#else
            LoadFonts(scale, s_fixedWidth, s_smallFont, s_bigFont);
            zigzagTex = (void*)(intptr_t)0;

            tracy::Config config;
            config.threadedRendering = true;
            config.targetFps = refreshRate;
            mTracyServer = std::make_unique<tracy::View>( RunOnMainThread, "127.0.0.1", 8086, s_fixedWidth, s_smallFont, s_bigFont, nullptr, nullptr, AttentionCallback, config);
            mTracyServer->GetViewData().frameTarget = refreshRate;
            mTracyServer->GetViewData().drawFrameTargets = true;
            mTracyServer->GetViewData().drawCpuUsageGraph = false;

            mCPUFrametime.reserve(1000);
            mCPUFrametimeMax = 0.f;
            mGPUFrametime.reserve(1000);
            mGPUFrametimeMax = 0.f;
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

            tracy::MouseFrame();
        }

        void Profiler::imGuiEditor() const {
#ifndef NO_LOCAL_TRACY
            NEO_ASSERT(mTracyServer, "Tracy server doesn't exist..?");

            // Profiler is baked into the viewport dock space
            mTracyServer->Draw();

            // Also have another simple graph for when the tracy profiler is collapsed
            {
                float latestCPUTime = mTracyServer->GetFrametime();
                mCPUFrametimeMax = std::max(mCPUFrametimeMax, latestCPUTime);
                if (mCPUFrametime.size() > 1000) {
                    mCPUFrametime.erase(mCPUFrametime.begin());
                }
                mCPUFrametime.emplace_back(latestCPUTime);
            }
            {
                float latestGPUTime = mTracyServer->GetGPUFrametime();
                mGPUFrametimeMax = std::max(mGPUFrametimeMax, latestGPUTime);
                if (mGPUFrametime.size() > 1000) {
                    mGPUFrametime.erase(mGPUFrametime.begin());
                }
                mGPUFrametime.emplace_back(latestGPUTime);
            }

            ImGui::Begin("BasicProfiler");
            if (ImPlot::BeginPlot(std::string("FPS (" + std::to_string(mTracyServer->GetFPS()) + ")").c_str())) {
                ImPlot::SetupAxis(ImAxis_X1, "Time (s)", ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoInitialFit  );
                ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoInitialFit | ImPlotAxisFlags_NoLabel );
                // ImPlot::SetupAxisLimits(ImAxis_Y1, 0, std::max(mCPUFrametimeMax, mGPUFrametimeMax) + 3.f, ImPlotCond_Always);
                ImPlot::PlotLine("CPU", mCPUFrametime.data(), static_cast<int>(mCPUFrametime.size()));
                ImPlot::PlotLine("GPU", mGPUFrametime.data(), static_cast<int>(mGPUFrametime.size()));
                ImPlot::EndPlot();
            }
            ImGui::End();
            // ImGui::Text("dt: %0.3f", mTracyServer->GetFrametime());
            // ImGui::Text("gdt: %0.3f", mTracyServer->GetGPUFrametime());
#endif
        }
    }
}