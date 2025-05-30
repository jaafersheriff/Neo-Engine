#include "ECS/pch.hpp"
#include "ECS.hpp"

#include "Component/EngineComponents/PinnedComponent.hpp"
#include "Component/EngineComponents/TagComponent.hpp"
#include "Component/CollisionComponent/SelectedComponent.hpp"

namespace neo {

	void ECS::_initSystems() {
		for (auto& system : mSystems) {
			system.second->init(*this);
		}
	}

	void ECS::_updateSystems(const ResourceManagers& resourceManagers) {
		TRACY_ZONEN("Update Systems");
		for (auto& system : mSystems) {
			if (system.second->mActive) {
				system.second->update(*this, resourceManagers);
			}
		}
	}

	void ECS::submitEntity(EntityBuilder&& builder) {
		std::lock_guard<std::mutex> lock(mEntityCreationMutex);
		mEntityCreateQueue.push_back(builder);
	}

	void ECS::removeEntity(Entity e) {
		std::lock_guard<std::mutex> lock(mEntityKillMutex);
		mEntityKillQueue.push_back(e);
	}

	void ECS::_flush() {
		TRACY_ZONE();

		{
			TRACY_ZONEN("Create Entities");
			std::vector<EntityBuilder> swapQueue;
			{
				std::lock_guard<std::mutex> lock(mEntityCreationMutex);
				std::swap(swapQueue, mEntityCreateQueue);
				mEntityCreateQueue.clear();
			}
			for (auto&& builder : swapQueue) {
				auto entity = mRegistry.create();
				for (auto&& job : builder.mComponents) {
					job(*this, entity);
				}
			}
		}

		{
			TRACY_ZONEN("Add Component");
			std::vector<ComponentModFunc> swapQueue;
			{
				std::lock_guard<std::mutex> lock(mAddComponentMutex);
				std::swap(swapQueue, mAddComponentFuncs);
				mAddComponentFuncs.clear();
			}
			for (auto&& job : swapQueue) {
				job(mRegistry);
			}
		}

		{
			TRACY_ZONEN("Remove Component");
			std::vector<ComponentModFunc> swapQueue;
			{
				std::lock_guard<std::mutex> lock(mRemoveComponentMutex);
				std::swap(swapQueue, mRemoveComponentFuncs);
				mRemoveComponentFuncs.clear();
			}
			for (auto&& job : swapQueue) {
				job(mRegistry);
			}
		}
		{
			TRACY_ZONEN("Kill Entities");
			std::vector<Entity> swapQueue;
			{
				std::lock_guard<std::mutex> lock(mEntityKillMutex);
				std::swap(swapQueue, mEntityKillQueue);
				mEntityKillQueue.clear();
			}
			mRegistry.destroy(swapQueue.cbegin(), swapQueue.cend());
		}
	}

	void ECS::_clean() {
		NEO_LOG_I("Cleaning ECS...");
		_flush();
		mRegistry.each([this](auto entity) {
			mRegistry.destroy(entity);
		});
		mRegistry.clear();
		NEO_ASSERT(mRegistry.alive() == 0, "What");
		mSystems.clear();
	}

	void ECS::_imguiEdtor() {
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
		ImGui::End();
	}
}
