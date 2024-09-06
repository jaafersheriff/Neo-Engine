#include "ECS/pch.hpp"
#include "ECS.hpp"

#include "Component/EngineComponents/PinnedComponent.hpp"
#include "Component/EngineComponents/TagComponent.hpp"
#include "Component/CollisionComponent/SelectedComponent.hpp"

namespace neo {

	// ECS::ECS(const ECS& other) {
	// 	TRACY_ZONE();
	// 	other.clone(*this);
	// }

	void ECS::_initSystems() {
		for (auto& system : mSystems) {
			system.second->init(*this);
		}
	}

	void ECS::_updateSystems() {
		TRACY_ZONEN("Update Systems");
		for (auto& system : mSystems) {
			if (system.second->mActive) {
				system.second->update(*this);
			}
		}
	}

	void ECS::clone(ECS& otherECS) const {
		TRACY_ZONE();

		using namespace entt::literals;
		mRegistry.each([&](ECS::Entity srcEntity) {
			auto newEntity = otherECS.createEntity();
			for (auto [id, storage] : mRegistry.storage()) {
				if (auto it = otherECS.mRegistry.storage(id); it != otherECS.mRegistry.storage().end() && storage.contains(srcEntity)) {
					it->second.emplace(newEntity, storage.get(srcEntity));
				}
			}
		});
	}

	ECS::Entity ECS::createEntity() {
		// TODO - this might break while threading
		return mRegistry.create();
	}

	void ECS::removeEntity(Entity e) const {
		std::lock_guard<std::mutex> lock(mEntityKillMutex);
		mEntityKillQueue.push_back(e);
	}

	void ECS::flush() {
		TRACY_ZONE();
		for (auto&& job : mAddComponentFuncs) {
			job(mRegistry);
		}
		mAddComponentFuncs.clear();

		for (auto&& job : mRemoveComponentFuncs) {
			job(mRegistry);
		}
		mRemoveComponentFuncs.clear();

		std::lock_guard<std::mutex> lock(mEntityKillMutex);
		mRegistry.destroy(mEntityKillQueue.cbegin(), mEntityKillQueue.cend());
		mEntityKillQueue.clear();
	}

	void ECS::clean() {
		NEO_LOG_I("Cleaning ECS...");
		flush();
		mRegistry.each([this](auto entity) {
			mRegistry.destroy(entity);
			});
		mRegistry.clear();
		mSystems.clear();
	}

	void ECS::imguiEdtor() {
		TRACY_ZONE();
		ImGui::Begin("ECS");
		auto pinnedView = getView<PinnedComponent>();
		if (!pinnedView.empty() && ImGui::TreeNodeEx("Pinned Entities", ImGuiTreeNodeFlags_DefaultOpen)) {
			pinnedView.each([this](Entity entity, PinnedComponent&) {
				char title[64];
				if (has<TagComponent>(entity)) {
					sprintf(title, "%s", getComponent<TagComponent>(entity)->mTag.c_str());
				}
				else {
					sprintf(title, "%d", static_cast<int>(entity));
				}
				if (ImGui::TreeNodeEx(title)) {
					mEditor.renderEditor(mRegistry, entity);
					ImGui::TreePop();
				}
				});
			ImGui::TreePop();
		}
		auto selected = getComponent<SelectedComponent>();
		if (selected.has_value()) {
			auto&& [selectedEntity, _] = *selected;
			char title[64];
			if (has<TagComponent>(selectedEntity)) {
				sprintf(title, "Selected: %s", getComponent<TagComponent>(selectedEntity)->mTag.c_str());
			}
			else {
				sprintf(title, "Selected: %d", static_cast<int>(selectedEntity));
			}
			if (ImGui::TreeNodeEx(title, ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_DefaultOpen)) {
				mEditor.renderEditor(mRegistry, selectedEntity);
				ImGui::TreePop();
			}
		}
		if (ImGui::TreeNodeEx(&mRegistry, 0, "All Entities: %d", static_cast<int>(mRegistry.alive()))) {
			getView<TagComponent>().each([this](Entity entity, TagComponent& tag) {
				if (ImGui::TreeNodeEx(tag.mTag.c_str())) {
					mEditor.renderEditor(mRegistry, entity);
					ImGui::TreePop();
				}
			});
			ImGui::TreePop();
		}

		if (mSystems.size() && ImGui::TreeNodeEx("Systems", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (unsigned i = 0; i < mSystems.size(); i++) {
				auto& sys = mSystems[i].second;
				ImGui::PushID(i);
				bool treeActive = ImGui::TreeNodeEx(sys->mName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
					ImGui::SetDragDropPayload("SYSTEM_SWAP", &i, sizeof(unsigned));
					ImGui::Text("Swap %s", sys->mName.c_str());
					ImGui::EndDragDropSource();
				}
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("SYSTEM_SWAP")) {
						IM_ASSERT(payLoad->DataSize == sizeof(unsigned));
						unsigned payload_n = *(const unsigned*)payLoad->Data;
						mSystems[i].swap(mSystems[payload_n]);
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::PopID();
				if (treeActive) {
					ImGui::Checkbox("Active", &sys->mActive);
					sys->imguiEditor(*this);
					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}

		if (mRenderECS) {
			if (ImGui::TreeNode(mRenderECS, "Render ECS")) {
				mRenderECS->imguiEdtor();
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}
}
