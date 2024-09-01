
#pragma once

#define IMGUI_USER_CONFIG "imgui_config.hpp"


#include "Util/Util.hpp"
#define IM_ASSERT(_EXPR) NEO_ASSERT(_EXPR, "ImGui Failed")

#define ImTextureID std::uint64_t

#include <imgui.h>
