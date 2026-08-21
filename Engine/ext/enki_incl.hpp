#pragma once

#include "Util/Assert.hpp"

#define ENKI_ASSERT(condition) do { NEO_ASSERT(condition, "enkiTS: %s", #condition); } while (false)

#include <TaskScheduler.h>
