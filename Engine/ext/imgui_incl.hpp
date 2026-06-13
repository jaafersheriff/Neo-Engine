
#pragma once

#include <ext/imgui_config.hpp>
#include <imgui.h>

// #include <algorithm> // For std::clamp
// #include <cmath>     // For std::log2
// #include <cstdio>    // For std::snprintf

namespace ImGui {

    template<typename T,
        typename std::enable_if_t<std::is_integral_v<T>, bool> = true>
    bool SliderPowerOfTwo(const char* label, T* v_current, int v_min, int v_max) {
		NEO_ASSERT(v_min <= v_max, "Invalid range");
		int tmp =  std::clamp(static_cast<int>(*v_current), v_min, v_max);

        int exp_current = static_cast<int>(std::log2(tmp));
        int exp_min = static_cast<int>(std::log2(v_min));
        int exp_max = static_cast<int>(std::log2(v_max));

        char format_buf[32];
        std::snprintf(format_buf, sizeof(format_buf), "%d", tmp);
        bool changed = ImGui::SliderInt(label, &exp_current, exp_min, exp_max, format_buf);

        if (changed) {
            *v_current = static_cast<T>(1 << exp_current);
        }

        return changed;

    }
}
