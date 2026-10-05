#pragma once

#include <cstddef>
#include <cstdint>

#ifndef NEO_LOG_S
	#define NEO_LOG_S(severity, fmt, ...) neo::util::_log(severity, __FUNCTION__, fmt, __VA_ARGS__)
	#define NEO_LOG(fmt, ...) NEO_LOG_S(neo::util::LogSeverity::Info, fmt, __VA_ARGS__)
	#define NEO_LOG_V(fmt, ...) NEO_LOG_S(neo::util::LogSeverity::Verbose, fmt, __VA_ARGS__)
	#define NEO_LOG_I(fmt, ...) NEO_LOG_S(neo::util::LogSeverity::Info, fmt, __VA_ARGS__)
	#define NEO_LOG_W(fmt, ...) NEO_LOG_S(neo::util::LogSeverity::Warning, fmt, __VA_ARGS__)
	#define NEO_LOG_E(fmt, ...) NEO_LOG_S(neo::util::LogSeverity::Error, fmt, __VA_ARGS__)
#endif


// TODO - should these be under some private namespace?
namespace neo {
	namespace util {

#ifdef DEBUG_MODE
		static bool sLogVerbose = true;
#else
		static bool sLogVerbose = false;
#endif 
		static bool sLogInfo = true;
		static bool sLogWarning = true;
		static bool sLogError = true;

		enum class LogSeverity {
			Verbose,
			Info,
			Warning,
			Error
		};

		struct LogColor {
			float r, g, b;
		};
		LogColor logSeverityColor(LogSeverity severity);

		void _log(LogSeverity severity, const char* sig, const char* format, ...);

		// Formats a byte count into the largest unit it fits, for resource panels.
		void stringifyByteSize(uint32_t byteSize, char* outStr, size_t outStrSize);
	}
}
