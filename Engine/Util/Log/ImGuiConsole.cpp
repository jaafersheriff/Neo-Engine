#include "ImGuiConsole.hpp"

#include "Util/Util.hpp"

#include <ctype.h>

namespace neo {

    ImGuiConsole::ImGuiConsole() {
        clearLog();
        memset(mInputBuffer, 0, sizeof(mInputBuffer));
        mHistoryPos = -1;

        mAutoScrollEnabled = true;
        mScrollToBottom = false;
    }

    ImGuiConsole::~ImGuiConsole() {
        clearLog();
        for (int i = 0; i < mHistory.size(); i++) {
            free(mHistory[i]);
        }
    }

    void ImGuiConsole::clearLog() {
        for (int i = 0; i < mLogs.size(); i++) {
            free(mLogs[i].second);
        }
        mLogs.clear();
    }

    void ImGuiConsole::addLog(const char* log, util::LogSeverity severity) {
        NEO_ASSERT(log, "Trying to log nothing"); 
        size_t len = strlen(log) + 1; 
        void* buf = malloc(len); 
        memcpy(buf, (const void*)log, len);
        mLogs.push_back({ severity, static_cast<char*>(buf) });
        while (!mInfiniteLog && mLogs.size() > mMaxLogSize) {
            mLogs.erase(mLogs.begin());
        }
    }

    void ImGuiConsole::imGuiEditor() {
        if (!ImGui::Begin("Console"))
        {
            ImGui::End();
            return;
        }

        if (ImGui::Button("Clear")) { 
            clearLog(); 
        }
        ImGui::SameLine();
        bool copy_to_clipboard = ImGui::Button("Copy");

        // Options menu
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &mAutoScrollEnabled);

        ImGui::SameLine();
        ImGui::Checkbox("Infinite Scroll", &mInfiniteLog);
        if (!mInfiniteLog) {
            ImGui::InputInt("Max size", &mMaxLogSize, 1, 10, ImGuiInputTextFlags_CharsDecimal);
            mMaxLogSize = std::min(1000, std::max(mMaxLogSize, 1));
        }

        ImGui::Separator();

        // Reserve enough left-over height for 1 separator + 1 input text
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::Selectable("Clear")) {
                clearLog();
            }
            ImGui::EndPopup();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
        if (copy_to_clipboard) {
            ImGui::LogToClipboard();
        }

        for (int i = 0; i < mLogs.size(); i++) {
            auto&& [severity, log] = mLogs[i];
            if (!mFilter.PassFilter(log)) {
                continue;
            }

            // Normally you would store more information in your item than just a string.
            // (e.g. make Items[] an array of structure, store color/type etc.)
            glm::vec3 color = util::sLogSeverityData.at(severity).second;
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.x, color.y, color.z, 1.0));
            ImGui::TextWrapped(log);
            ImGui::PopStyleColor();
        }

        if (copy_to_clipboard) {
            ImGui::LogFinish();
        }

        if (mScrollToBottom || (mAutoScrollEnabled && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())) {
            ImGui::SetScrollHereY(1.0f);
        }
        mScrollToBottom = false;

        ImGui::PopStyleVar();
        ImGui::EndChild();

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();

        ImGui::End();
    }
}
