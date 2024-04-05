#include "ECS/pch.hpp"
#include "ECS.hpp"

#include "Component/EngineComponents/TagComponent.hpp"
#include "Component/CollisionComponent/SelectedComponent.hpp"

namespace neo {

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

	ECS::Entity ECS::createEntity() {
		// TODO - this might break while threading
		return mRegistry.create();
	}

	void ECS::removeEntity(Entity e) {
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
		NEO_ASSERT(mRegistry.alive() == 0, "What");
		mSystems.clear();
	}

	void ECS::imguiEdtor() {
		TRACY_ZONE();
		ImGui::Begin("ECS");
		char buf[64];
		sprintf(buf, "Entities: %d", static_cast<int>(mRegistry.alive()));
		if (ImGui::TreeNodeEx(buf, ImGuiTreeNodeFlags_DefaultOpen)) {
			getView<TagComponent>().each([this](Entity entity, TagComponent& tag) {
				if (ImGui::TreeNodeEx(tag.mTag.c_str(), has<SelectedComponent>(entity) ? ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_DefaultOpen : 0 )) {
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
