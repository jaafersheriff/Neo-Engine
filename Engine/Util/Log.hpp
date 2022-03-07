#pragma once

#include <vector>


#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#ifndef NEO_LOG_S
	#define NEO_LOG_S(severity, fmt, ...) neo::util::_log(severity, fmt, __VA_ARGS__)
	#define NEO_LOG(fmt, ...) NEO_LOG_S(neo::util::LogSeverity::Verbose, fmt, __VA_ARGS__)
	#define NEO_LOG_V(fmt, ...) NEO_LOG_S(neo::util::LogSeverity::Verbose, fmt, __VA_ARGS__)
	#define NEO_LOG_I(fmt, ...) NEO_LOG_S(neo::util::LogSeverity::Info, fmt, __VA_ARGS__)
	#define NEO_LOG_W(fmt, ...) NEO_LOG_S(neo::util::LogSeverity::Warning, fmt, __VA_ARGS__)
	#define NEO_LOG_E(fmt, ...) NEO_LOG_S(neo::util::LogSeverity::Error, fmt, __VA_ARGS__)
#endif // NEO_LOG


// TODO - should these be under some private namespace?
namespace neo {
    namespace util {

#ifdef DEBUG_MODE
        static bool sLogVerbose = true;
        static bool sLogInfo = true;
        static bool sLogWarning = true;
#else
        static bool sLogVerbose = false;
        static bool sLogInfo = false;
        static bool sLogWarning = false;
#endif // NEO_CONFIG_DEBUG
        static bool sLogError = true;

        enum class LogSeverity {
            Verbose,
            Info,
            Warning,
            Error
        };

        void _log(LogSeverity severity, const char* format, ...);

    }
}