#pragma once

#include "Util/Assert.hpp"

// The single sanctioned entrance to enkiTS. Include this instead of <TaskScheduler.h> - and never
// include <TaskScheduler.h> without it - so that every translation unit agrees on the configuration
// below.
//
// Why a wrapper header rather than a compile definition: ENKI_ASSERT is a *function-like* macro, and
// MSVC's /D cannot define those (unlike GCC), so target_compile_definitions is not an option. This
// follows the pattern ext/entt_incl.hpp and ext/imgui_incl.hpp already establish for ENTT_ASSERT and
// IM_ASSERT. Wrapped in a do/while so it stays a single statement - NEO_ASSERT expands to a bare
// `if` with no else, which would otherwise be capturable by a surrounding if/else.
//
// TaskScheduler.h only supplies its own fallback under `#ifndef ENKI_ASSERT`, so getting in first is
// all it takes.
#define ENKI_ASSERT(condition) do { NEO_ASSERT(condition, "enkiTS: %s", #condition); } while (false)

// One difference from entt_incl.hpp worth knowing about: EnTT is header-only, so routing its assert
// here covers all of it. enkiTS is not - TaskScheduler.cpp is compiled into the enkiTS static
// library, which never sees this header, so the scheduler's *internal* assertions stay on plain
// assert(). What this header covers is every assertion in the inline code Neo instantiates. Routing
// the library internals too would mean force-including a config header into the ext target (/FI),
// which is not worth a second header until an enkiTS-internal assert actually fires.
//
// ENKITS_TASK_PRIORITIES_NUM deliberately stays a compile definition on the enkiTS target
// (Engine/ext/CMakeLists.txt) rather than moving here. It sizes arrays inside TaskScheduler, so a TU
// that somehow reached enkiTS without this header would be an ODR violation and silent memory
// corruption, where the same mistake with ENKI_ASSERT only costs a missed assertion. /D covers every
// TU unconditionally; a header cannot.
#include <TaskScheduler.h>
