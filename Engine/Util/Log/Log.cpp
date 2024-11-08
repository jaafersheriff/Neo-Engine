#include "Util/pch.hpp"

#include "Log.hpp"

#include "Engine/ImGuiManager.hpp"

#include "Util/ServiceLocator.hpp"
#include "Util/Util.hpp"

#include <GLFW/glfw3.h>

#define ARRAYSIZE(_ARR)	((int)(sizeof(_ARR) / sizeof(*(_ARR))))	 // Size of a static C-style array. Don't use on pointers!

namespace neo {
	namespace util {

		void _log(LogSeverity severity, const char* sig, const char* format, ...) {
			static_assert(std::is_same<neo::util::LogSeverity, decltype(severity)>::value, "Invalid log severity");
			bool doTheLog = false;
			doTheLog |= severity == neo::util::LogSeverity::Verbose && neo::util::sLogVerbose;
			doTheLog |= severity == neo::util::LogSeverity::Info && neo::util::sLogInfo;
			doTheLog |= severity == neo::util::LogSeverity::Warning && neo::util::sLogWarning;
			doTheLog |= severity == neo::util::LogSeverity::Error && neo::util::sLogError;
			if (doTheLog) {

				char inbuf[2048];
				va_list args;
				va_start(args, format);
				vsnprintf(inbuf, ARRAYSIZE(inbuf), format, args);
				inbuf[ARRAYSIZE(inbuf) - 1] = 0;
				va_end(args);
				char buf[2048];

				if (severity != neo::util::LogSeverity::Error) {
					sprintf(buf, "%0.4f [%c] (%s): %s\n", glfwGetTime(), sLogSeverityData.at(severity).first, sig, inbuf);
				}
				else {
					sprintf(buf, "%0.4f [%c]: %s\n", glfwGetTime(), sLogSeverityData.at(severity).first, inbuf);
				}

#ifdef DEBUG_MODE
				fprintf(stderr, buf);
#endif
				if (ServiceLocator<ImGuiManager>::has_value()) {
					ServiceLocator<ImGuiManager>::value().log(buf, severity);
				}
			}
		}
	}
}
