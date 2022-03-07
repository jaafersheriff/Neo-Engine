#include "Log.hpp"

#include "Engine/ImGuiManager.hpp"

#include <iostream>
#include <sstream>
#include <map>
#include <imgui/imgui.h>

#include <GLFW/glfw3.h>

namespace neo {
	namespace util {

        const static std::map<LogSeverity, const char*> sLogSeverityString{
            { LogSeverity::Verbose, "Verbose" },
            { LogSeverity::Info, "Info" },
            { LogSeverity::Warning, "Warning" },
            { LogSeverity::Error, "Error" },
        };


		void _log(LogSeverity severity, const char* format, ...) {
			static_assert(std::is_same<neo::util::LogSeverity, decltype(severity)>::value, "Invalid log severity");
			bool _doTheLog = false;
			_doTheLog |= severity == neo::util::LogSeverity::Verbose && neo::util::sLogVerbose;
			_doTheLog |= severity == neo::util::LogSeverity::Info && neo::util::sLogInfo;
			_doTheLog |= severity == neo::util::LogSeverity::Warning && neo::util::sLogWarning;
			_doTheLog |= severity == neo::util::LogSeverity::Error && neo::util::sLogError;
			if (_doTheLog) {

				char inbuf[1024];
				va_list args;
				va_start(args, format);
				vsnprintf(inbuf, IM_ARRAYSIZE(inbuf), format, args);
				inbuf[IM_ARRAYSIZE(inbuf) - 1] = 0;
				va_end(args);

				char buf[1024];
				sprintf(buf, "%0.4f [%s]: %s", glfwGetTime(), sLogSeverityString.at(severity), inbuf);

				ImGuiManager::mConsole.AddLog(buf);
			}
		}
	}
}
