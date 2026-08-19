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
		mRegistry.clear();
		NEO_ASSERT(mRegistry.storage<Entity>().free_list() == 0, "What");
		mComponentRegistry._clear();
		mSystems.clear();
	}

	void ECS::_cloneInto(ECS& dst) const {
		TRACY_ZONE();

		{
			TRACY_ZONEN("Entities");
			// see entt/entity/snapshot.hpp. Walking rbegin()..rend() covers live AND released slots, so
			// indices, versions and the free list all come out matching.
			const Registry::common_type& srcEntities = mRegistry.storage<Entity>();
			auto& dstEntities = dst.mRegistry.storage<Entity>();
			dstEntities.clear();
			dstEntities.reserve(srcEntities.size());

			Entity placeholder{};
			for (auto it = srcEntities.rbegin(), last = srcEntities.rend(); it != last; ++it) {
				dstEntities.generate(*it);
				placeholder = (*it > placeholder) ? *it : placeholder;
			}
			dstEntities.start_from(entt::entt_traits<Entity>::next(placeholder));
			dstEntities.free_list(srcEntities.free_list());
		}

		{
			TRACY_ZONEN("Components");
			// Every component type that has ever been attached is in here, so demo-local types are
			// covered without anything to opt into.
			for (const auto& [type, entry] : mComponentRegistry) {
				NEO_UNUSED(type);
				entry.mClone(mRegistry, dst.mRegistry);
			}
		}

#if 0
		// The component clone pairs two independent iterators - the packed entity array and the packed
		// element array - and a mispairing would silently associate components with the wrong entities
		// rather than crash. Check structurally that the clone really mirrors the source.
		// This is O(entities + components) on top of the clone itself; drop it once the clone has been
		// exercised across every demo.
		{
			TRACY_ZONEN("Validate clone");

			const Registry::common_type& srcEntities = mRegistry.storage<Entity>();
			const Registry::common_type& dstEntities = dst.mRegistry.storage<Entity>();
			NEO_ASSERT(srcEntities.size() == dstEntities.size(), "Clone entity pool size mismatch");
			NEO_ASSERT(srcEntities.free_list() == dstEntities.free_list(), "Clone live entity count mismatch");
			for (auto s = srcEntities.rbegin(), d = dstEntities.rbegin(); s != srcEntities.rend(); ++s, ++d) {
				NEO_ASSERT(*s == *d, "Clone entity order mismatch - identifiers did not survive");
			}

			for (const auto& [type, entry] : mComponentRegistry) {
				NEO_UNUSED(entry);
				const Registry::common_type* from = mRegistry.storage(type);
				const Registry::common_type* to = dst.mRegistry.storage(type);
				if (from == nullptr) {
					continue;
				}
				NEO_ASSERT(to != nullptr, "Clone is missing a pool that the source has");
				NEO_ASSERT(from->size() == to->size(), "Clone pool size mismatch");
				for (auto s = from->rbegin(), d = to->rbegin(); s != from->rend(); ++s, ++d) {
					NEO_ASSERT(*s == *d, "Clone pool entity order mismatch");
				}
			}
		}
#endif
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
					_imguiComponentEditor(entity);
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
				_imguiComponentEditor(selectedEntity);
				ImGui::TreePop();
			}
		}
		if (ImGui::TreeNodeEx(&mRegistry, 0, "All Entities: %d", static_cast<int>(mRegistry.storage<Entity>().free_list()))) {
			getView<TagComponent>().each([this](Entity entity, TagComponent& tag) {
				if (ImGui::TreeNodeEx(tag.mTag.c_str())) {
					_imguiComponentEditor(entity);
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

	void ECS::_imguiComponentEditor(Entity e) {
		if (!mRegistry.valid(e)) {
			ImGui::TextUnformatted("Invalid Entity");
			return;
		}

		ImGui::PushID(static_cast<int>(entt::to_integral(e)));

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.15f, 0.15f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.f, 0.2f, 0.2f, 1.f));
		if (ImGui::Button("Destroy Entity")) {
			removeEntity(e);
		}
		ImGui::PopStyleColor(3);
		ImGui::Separator();

		for (const auto& [type, entry] : mComponentRegistry) {
			const auto* storage = mRegistry.storage(type);
			if (!storage || !storage->contains(e)) {
				continue;
			}

			ImGui::PushID(type);
			if (ImGui::Button("-")) {
				entry.mRemove(*this, e);
				ImGui::PopID();
				continue; // Early out - the component is queued for removal
			}
			ImGui::SameLine();

			if (ImGui::CollapsingHeader(entry.mName)) {
				ImGui::Indent(30.f);
				ImGui::PushID("Widget");
				entry.mWidget(*this, e);
				ImGui::PopID();
				ImGui::Unindent(30.f);
			}
			ImGui::PopID();
		}

		ImGui::PopID();
	}
}
