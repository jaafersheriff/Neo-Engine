
#pragma once


#include "Util/Util.hpp"
#define IM_ASSERT(_EXPR) do { NEO_ASSERT(_EXPR, "ImGui Failed"); } while (0)
#define IM_DEBUG_BREAK() do { NEO_FAIL("ImGui Failed"); } while (0)

#define ImTextureID std::uint64_t // matches entt::id_type

