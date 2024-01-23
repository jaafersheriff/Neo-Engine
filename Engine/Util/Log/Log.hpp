#pragma once

#include <glm/glm.hpp>
#include <map>

#ifndef NEO_LOG_S
	#define NEO_LOG_S(severity, fmt, ...) neo::util::_log(severity, __FUNCSIG__, fmt, __VA_ARGS__)
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

        const static std::map<LogSeverity, std::pair<char, glm::vec3>> sLogSeverityData {
            { LogSeverity::Verbose, {'V', glm::vec3(0.26f, 0.4f, 0.32f)}},
            { LogSeverity::Info,    {'I', glm::vec3(0.09f, 0.67f, 0.39f)}},
            { LogSeverity::Warning, {'W', glm::vec3(1.0f, 1.0f, 0.0f)}},
            { LogSeverity::Error,   {'E', glm::vec3(1.0f, 0.2f, 0.2f)}},
        };

        void _log(LogSeverity severity, const char* sig, const char* format, ...);
    }
}