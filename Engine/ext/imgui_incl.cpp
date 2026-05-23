
#include "ext/imgui_incl.hpp"

#include <algorithm>

namespace ImGui {

    bool SliderPowerOfTwo(const char* label, int* v_current, int v_min, int v_max) {
		NEO_ASSERT(v_min <= v_max, "Invalid range");
        *v_current = std::clamp(*v_current, v_min, v_max);

        int exp_current = static_cast<int>(std::log2(*v_current));
        int exp_min = static_cast<int>(std::log2(v_min));
        int exp_max = static_cast<int>(std::log2(v_max));

        char format_buf[32];
        std::snprintf(format_buf, sizeof(format_buf), "%d", *v_current);
        bool changed = ImGui::SliderInt(label, &exp_current, exp_min, exp_max, format_buf);

        if (changed) {
            *v_current = 1 << exp_current;
        }

        return changed;
    }
}
