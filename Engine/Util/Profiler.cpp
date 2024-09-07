#include "Util/pch.hpp"

#include "Util/Util.hpp"

#include "Profiler.hpp"

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
	namespace util {

		Profiler::Profiler() {
		}

		Profiler::~Profiler() {
		}

		void Profiler::begin(double _runTime) {
			TRACY_ZONE();
			mFrame++;
			/* Update delta time and FPS */
			float runTime = static_cast<float>(_runTime);
			mTimeStep = runTime - mLastFrameTime;
			mLastFrameTime = runTime;
		}

		void Profiler::end(double _runTime) {
			NEO_UNUSED(_runTime);
			NEO_LOG_E("TODO");
		}

		void Profiler::imGuiEditor() const {
			ImGui::Begin("Profiler");
			ImGui::Text("TODO");
			ImGui::End();
		}
	}
}
