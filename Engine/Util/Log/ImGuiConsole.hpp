#pragma once

#include "Util/Log/Log.hpp"

#include <vector>

#include <imgui_incl.hpp>

namespace neo {

	class ImGuiConsole {
	public:
		ImGuiConsole();
		~ImGuiConsole();

		void clearLog();
		void addLog(const char* log, util::LogSeverity severity);
		void imGuiEditor();

	private:
		std::vector<std::pair<util::LogSeverity, char*>> mLogs;
		ImGuiTextFilter mFilter;
		bool mAutoScrollEnabled;
		bool mScrollToBottom;
		int mMaxLogSize = 100;
		bool mInfiniteLog = false;
	};

}
