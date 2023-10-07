#pragma once

#include "Util/Log/Log.hpp"

#include <imgui/imgui.h>

namespace neo {

    class ImGuiConsole {
    public:
        ImGuiConsole();
        ~ImGuiConsole();

        void clearLog();
        void addLog(const char* log, util::LogSeverity severity);
        void imGuiEditor();

    private:
        char mInputBuffer[256];
        std::vector<std::pair<util::LogSeverity, char*>> mLogs;
        std::vector<char*> mHistory;
        int mHistoryPos;    // -1: new line, 0..History.size()-1 browsing history.
        ImGuiTextFilter mFilter;
        bool mAutoScrollEnabled;
        bool mScrollToBottom;
        int mMaxLogSize = 100;
        bool mInfiniteLog = false;
    };

}
