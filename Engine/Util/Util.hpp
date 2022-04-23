#pragma once

#include "Util/Log/Log.hpp"
#include "entt/core/hashed_string.hpp"

namespace neo {

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define NEO_UNUSED(...) __noop(__VA_ARGS__)
 
#ifndef NEO_DEBUG_ASSERT
	#ifdef DEBUG_MODE
		#define NEO_ASSERT(c, fmt, ...) \
			if (!(c)) { \
				NEO_LOG_E("ASSERT(%s) in %s, file %s on line %d", #c, __func__, __FILENAME__, __LINE__); \
				NEO_LOG_E(fmt, __VA_ARGS__); \
				abort(); \
			} 
		#define NEO_FAIL(fmt, ...) NEO_ASSERT(false, fmt, __VA_ARGS__)
	#else
		#define NEO_ASSERT(c, fmt, ...) \
			if (!(c)) { \
				NEO_LOG_E("ASSERT(%s) in %s, file %s on line %d", #c, __func__, __FILENAME__, __LINE__); \
				NEO_LOG_E(fmt, __VA_ARGS__); \
			} 
		#define NEO_FAIL(fmt, ...) NEO_UNUSED(fmt); NEO_UNUSED(__VA_ARGS__) ; abort()
	#endif // NEO_CONFIG_DEBUG
#endif // NEO_DEBUG_ASSERT

#define NEO_ARRAYSIZE(_ARR)		  ((int)(sizeof(_ARR) / sizeof(*(_ARR))))	 // Size of a static C-style array. Don't use on pointers!

	using HashedString = entt::hashed_string;

	namespace util {

		static inline bool fileExists(const char* fn);

		static inline char* textFileRead(const char* fn);

		static inline int textFileWrite(const char* fn, char* s);

	}
}