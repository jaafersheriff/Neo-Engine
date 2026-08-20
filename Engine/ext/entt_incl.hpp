#pragma once

#include "Util/Assert.hpp"

// The single sanctioned entrance to EnTT. Include this before any <entt/...> header - and never
// include <entt/...> without it - so that every translation unit agrees on the configuration below.
//
// Why a wrapper header rather than a compile definition: ENTT_ASSERT is a *function-like* macro, and
// MSVC's /D cannot define those (unlike GCC), so target_compile_definitions is not an option. This
// follows the pattern ext/imgui_incl.hpp already establishes for IM_ASSERT.
//
// entt/config/config.h only supplies its own fallback under `#elif !defined ENTT_ASSERT`, so getting
// in first is all it takes. Wrapped in a do/while so it stays a single statement - NEO_ASSERT expands
// to a bare `if` with no else, which would otherwise be capturable by a surrounding if/else.
#define ENTT_ASSERT(condition, msg) do { NEO_ASSERT(condition, "%s", msg); } while (false)

// Pulled in here, rather than left to whichever entt header the includer wants, so the definition
// above is locked in for the whole translation unit the moment this header is first seen. Because
// every entrance to EnTT goes through here, the macro is established before config.h regardless of
// which entrance a given TU hits first, and include order stops mattering.
//
// ENTT_ASSERT_CONSTEXPR is deliberately left alone: config.h defaults it to ENTT_ASSERT, so it
// inherits the routing above.
//
// ENTT_NO_ETO deliberately stays a compile definition on the entt target (Engine/ext/CMakeLists.txt)
// rather than moving here. It changes storage *layout*, so a TU that somehow reached EnTT without
// this header would be an ODR violation and silent memory corruption, where the same mistake with
// ENTT_ASSERT only costs a missed assertion. /D covers every TU unconditionally; a header cannot.
#include <entt/config/config.h>
