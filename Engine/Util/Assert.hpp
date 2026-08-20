#pragma once

#include "Util/Log/Log.hpp"

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
