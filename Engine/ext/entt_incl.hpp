#pragma once

#include "Util/Assert.hpp"

#define ENTT_ASSERT(condition, msg) do { NEO_ASSERT(condition, "%s", msg); } while (false)

#include <entt/config/config.h>
