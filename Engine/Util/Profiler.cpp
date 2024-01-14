#include "Util/pch.hpp"

#include "Util/Util.hpp"

#include "Profiler.hpp"

#ifndef NO_LOCAL_TRACY
#include <TracyView.hpp>
#include <TracyMouse.hpp>
#include <Fonts.hpp>
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
            ImGui::Begin("Basic Profiler");
            ImGui::Text("FPS: %0.3f", mTracyServer->GetFPS());
            ImGui::Text("dt: %0.3f", mTracyServer->GetFrametime());
            ImGui::Text("gdt: %0.3f", mTracyServer->GetGPUFrametime());
            ImGui::End();
#endif
        }
    }
}