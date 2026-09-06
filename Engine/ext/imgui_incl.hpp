
#pragma once

#include <ext/imgui_config.hpp>
#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <type_traits>

namespace ImGui {

	// Slides over the exponent while displaying the value, so a resolution or a brick size can only
	// ever land on a power of two.
	template<typename T, typename std::enable_if_t<std::is_integral_v<T>, bool> = true>
	bool SliderPowerOfTwo(const char* label, T* v_current, int v_min, int v_max) {
		NEO_ASSERT(v_min <= v_max, "Invalid range");
		const int clamped = std::clamp(static_cast<int>(*v_current), v_min, v_max);

		int expCurrent = static_cast<int>(std::log2(clamped));
		const int expMin = static_cast<int>(std::log2(v_min));
		const int expMax = static_cast<int>(std::log2(v_max));

		char formatBuf[32];
		std::snprintf(formatBuf, sizeof(formatBuf), "%d", clamped);
		const bool changed = ImGui::SliderInt(label, &expCurrent, expMin, expMax, formatBuf);
		if (changed) {
			*v_current = static_cast<T>(1 << expCurrent);
		}
		return changed;
	}
}
