
#pragma once

#include <ext/imgui_config.hpp>
#include <imgui.h>

namespace ImGui {

    bool SliderPowerOfTwo(const char* label, int* v_current, int v_min, int v_max);
}
