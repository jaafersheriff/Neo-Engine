#include "Log.hpp"

#include "Util/Util.hpp"

#include "Engine/ImGuiManager.hpp"

#include <iostream>
#include <sstream>
#include <map>
#include <imgui/imgui.h>

#include <GLFW/glfw3.h>

namespace neo {
	namespace util {

		void _log(LogSeverity severity, const char* format, ...) {
			static_assert(std::is_same<neo::util::LogSeverity, decltype(severity)>::value, "Invalid log severity");
			bool doTheLog = false;
			doTheLog |= severity == neo::util::LogSeverity::Verbose && neo::util::sLogVerbose;
			doTheLog |= severity == neo::util::LogSeverity::Info && neo::util::sLogInfo;
			doTheLog |= severity == neo::util::LogSeverity::Warning && neo::util::sLogWarning;
			doTheLog |= severity == neo::util::LogSeverity::Error && neo::util::sLogError;
			if (doTheLog) {

				char inbuf[1024];
				va_list args;
				va_start(args, format);
				vsnprintf(inbuf, NEO_ARRAYSIZE(inbuf), format, args);
				inbuf[NEO_ARRAYSIZE(inbuf) - 1] = 0;
				va_end(args);
				char buf[1024];
				sprintf(buf, "%0.4f [%c]: %s\n", glfwGetTime(), sLogSeverityData.at(severity).first, inbuf);

#ifdef DEBUG_MODE
				fprintf(stderr, buf);
#endif
				ImGuiManager::log(buf, severity);
			}
		}
	}
}