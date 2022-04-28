#include "ECS/pch.hpp"
#include "ECS.hpp"

#include "Component/EngineComponents/TagComponent.hpp"

#include <imgui_entt_entity_editor/imgui_entt_entity_editor.hpp>

namespace neo {

	void ECS::_initSystems() {
		for (auto& system : mSystems) {
			system.second->init(*this);
		}
	}

	void ECS::_updateSystems() {
		MICROPROFILE_SCOPEI("ECS", "_updateSystems", MP_AUTO);
		for (auto& system : mSystems) {
			if (system.second->mActive) {
				MICROPROFILE_DEFINE(Systems, "ECS", system.second->mName.c_str(), MP_AUTO);
				MICROPROFILE_ENTER(Systems);
				system.second->update(*this);
				MICROPROFILE_LEAVE();
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
        MICROPROFILE_SCOPEI("ECS", "flush", MP_AUTO);
		for (auto&& job : mAddComponentFuncs) {
			job(mRegistry);
		}
		mAddComponentFuncs.clear();

		for (auto&& job : mRemoveComponentFuncs) {
			job(mRegistry);
		}
		mRemoveComponentFuncs.clear();

		for (Entity e : mEntityKillQueue) {
			mRegistry.destroy(e);
		}
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
		ImGui::Begin("ECS", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove);
		size_t count = 0;
		// for (auto go : getGameObjects()) {
		// 	count += go->getAllComponents().size();
		// }
		ImGui::Text("Components: %d", count);
		char buf[256];
		sprintf(buf, "Entities: %d", static_cast<int>(mRegistry.size()));
		if (ImGui::TreeNodeEx(buf, ImGuiTreeNodeFlags_DefaultOpen)) {
			getView<TagComponent>().each([this](Entity entity, TagComponent& tag) {
				NEO_UNUSED(entity);
				if (ImGui::TreeNodeEx(tag.mTag.c_str())) {
					mEditor.renderEditor(mRegistry, entity);
					// for (auto&& [type, components] : gameObject->getComponentsMap()) {
					// 	for (int i = 0; i < components.size(); i++) {
					// 		ImGui::Text(type.name());
					// 		components[i]->imGuiEditor();
					// 		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.81f, 0.20f, 0.20f, 0.40f));
					// 		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.81f, 0.20f, 0.20f, 1.00f));
					// 		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.81f, 0.15f, 0.05f, 1.00f));
					// 		ImGui::PushID(gameObject + type.hash_code() + i);
					// 		if (ImGui::Button("Remove")) {
					// 			_removeComponent(type, components[i]);
					// 		}
					// 		ImGui::PopID();
					// 		ImGui::PopStyleColor(3);
					// 		ImGui::Separator();
					// 	}
					// }
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
