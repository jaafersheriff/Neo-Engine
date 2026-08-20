#pragma once

#include "Util/Log/Log.hpp"

// Neo's assertion macros, split out of Util.hpp so they can be used *below* it in the include graph.
// The specific reason they live here rather than there: ext/entt_incl.hpp has to have NEO_ASSERT in
// scope before it includes anything from EnTT, and Util.hpp cannot provide that - it pulls in
// entt/core/hashed_string.hpp itself, so depending on it would be a cycle. Nothing in this header
// may include EnTT, directly or transitively.

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define NEO_UNUSED(...) __noop(__VA_ARGS__)

#ifndef NEO_DEBUG_ASSERT
	#ifdef DEBUG_MODE
		#define NEO_ASSERT(c, fmt, ...) \
			if (!(c)) { \
				NEO_LOG_E("ASSERT(%s) in %s, file %s on line %d", #c, __func__, __FILENAME__, __LINE__); \
				NEO_LOG_E(fmt, __VA_ARGS__); \
				__debugbreak(); \
			} 
	#else
		#define NEO_ASSERT(c, fmt, ...) \
			if (!(c)) { \
				NEO_LOG_E("ASSERT(%s) in %s, file %s on line %d", #c, __func__, __FILENAME__, __LINE__); \
				NEO_LOG_E(fmt, __VA_ARGS__); \
			} 
	#endif // NEO_CONFIG_DEBUG
	#define NEO_FAIL(fmt, ...) NEO_ASSERT(false, fmt, __VA_ARGS__)
#endif // NEO_DEBUG_ASSERT
