#include "Util/pch.hpp"
#include "ImGuiConsole.hpp"

#include "Util/Util.hpp"
#include "Util/Profiler.hpp"

namespace neo {

	ImGuiConsole::ImGuiConsole() {
		clearLog();

		mAutoScrollEnabled = true;
		mScrollToBottom = false;
	}

	ImGuiConsole::~ImGuiConsole() {
		clearLog();
	}

	void ImGuiConsole::clearLog() {
		std::lock_guard<std::mutex> lock(mLogMutex);
		for (int i = 0; i < mLogs.size(); i++) {
			delete[] mLogs[i].second;
		}
		mLogs.clear();
	}

	void ImGuiConsole::addLog(const char* log, util::LogSeverity severity) {
		NEO_ASSERT(log, "Trying to log nothing"); 
		size_t len = strlen(log) + 1; 
		char* buf = new char[len]; 
		memcpy(buf, log, len);

		{
			std::lock_guard<std::mutex> lock(mLogMutex);
			mLogs.push_back({ severity, buf });
			while (!mInfiniteLog && mLogs.size() > mMaxLogSize) {
				delete[] mLogs.front().second;
				mLogs.erase(mLogs.begin());
			}
		}
	}

	void ImGuiConsole::imGuiEditor() {
		TRACY_ZONE();
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
		ImGui::BeginChild("ScrollingRegion");
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

		{
			std::lock_guard<std::mutex> lock(mLogMutex);
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
