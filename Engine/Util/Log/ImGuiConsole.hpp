#pragma once

#include "Util/Log/Log.hpp"

#include <vector>
#include <mutex>

#include <ext/imgui_incl.hpp>

namespace neo {

	class ImGuiConsole {
	public:
		ImGuiConsole();
		~ImGuiConsole();

		void clearLog();
		void addLog(const char* log, util::LogSeverity severity);
		void imGuiEditor();

	private:
		std::mutex mLogMutex;
		std::vector<std::pair<util::LogSeverity, char*>> mLogs;
		bool mAutoScrollEnabled;
		bool mScrollToBottom;
		int mMaxLogSize = 100;
		bool mInfiniteLog = false;
	};

}
