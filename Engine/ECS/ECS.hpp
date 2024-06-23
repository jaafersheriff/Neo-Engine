#pragma once

#include "ECS/Systems/System.hpp"

#include "Loader/Loader.hpp"

#include "Util/Profiler.hpp"

#ifndef ENTT_ASSERT
#define ENTT_ASSERT(condition, ...) NEO_ASSERT(condition, __VA_ARGS__)
#endif
#include <entt/entt.hpp>

#define MM_IEEE_ASSERT(x) NEO_UNUSED(x)
#include <imgui_entt_entity_editor.hpp>

#include <typeindex>

namespace neo {
	class System;
	class Engine;

	class ECS {
		friend Engine;

	public:
		ECS() = default;
		~ECS() = default;
		ECS(const ECS&) = delete;
		ECS& operator=(const ECS&) = delete;

		void flush();
		void clean();
		void imguiEdtor();

		/* ECS */
		using Entity = entt::entity;
		using Registry = entt::registry;

		Entity createEntity();
		void removeEntity(Entity e);

		// Entity access
		template<typename CompT, typename... Args> CompT* addComponent(Entity e, Args &&... args);
		template<typename CompT> void removeComponent(Entity e);

		template<typename CompT> bool has(Entity e) const;
		template<typename CompT> CompT* getComponent(Entity e);
		template<typename CompT> CompT *const cGetComponent(Entity e) const;

		// All access
		template<typename CompT> std::optional<std::tuple<Entity, CompT&>> getComponent();
		template<typename CompT> std::optional<std::tuple<ECS::Entity, const CompT&>> cGetComponent() const;
		template<typename... CompTs> bool has() const;
		template<typename... CompTs> auto getView();
		template<typename... CompTs> const auto getView() const;
		template<typename... CompTs> std::optional<std::tuple<Entity, CompTs&...>> getSingleView();
		template<typename... CompTs> std::optional<std::tuple<Entity, const CompTs&...>> getSingleView() const;

		template<typename Comp> void sort(std::function<bool(Entity left, Entity right)> compare) const;

		/* Attach a system */
		template <typename SysT, typename... Args> SysT& addSystem(Args &&...);
		template <typename SysT> bool isSystemEnabled() const;


	private:
		mutable Registry mRegistry;
		mutable MM::EntityEditor<Entity> mEditor;

		/* Active containers */
		std::vector<Entity> mEntityKillQueue;
		using ComponentModFunc = std::function<void(Registry&)>;
		std::vector<ComponentModFunc> mAddComponentFuncs;
		std::vector<ComponentModFunc> mRemoveComponentFuncs;

		std::vector<std::pair<std::type_index, std::unique_ptr<System>>> mSystems;
		void _initSystems();
		void _updateSystems();
	};

	template<typename CompT>
	std::optional<std::tuple<ECS::Entity, CompT&>> ECS::getComponent() {
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");
	
		auto view = mRegistry.view<CompT>();
		if (view.size() > 1) {
			NEO_LOG_E("Attempting to get a single %s when multiple exist", mRegistry.try_get<CompT>(view.front())->mName);
		}
		if (view.size()) {
			return { *view.each().begin() };
		}
		return std::nullopt;
	}

	template<typename CompT>
	std::optional<std::tuple<ECS::Entity, const CompT&>> ECS::cGetComponent() const{
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");
	
		auto view = mRegistry.view<CompT>();
		if (view.size() > 1) {
			NEO_LOG_E("Attempting to get a single %s when multiple exist", mRegistry.try_get<CompT>(view.front())->mName);
		}
		if (view.size()) {
			return { *view.each().begin() };
		}
		return std::nullopt;
	}

	template<typename CompT>
	bool ECS::has(ECS::Entity e) const {
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

		return mRegistry.try_get<CompT>(e) != nullptr;
	}

	template <typename SysT, typename... Args> 
	SysT& ECS::addSystem(Args &&... args) {
		static_assert(std::is_base_of<System, SysT>::value, "SysT must be a System type");
		static_assert(!std::is_same<SysT, System>::value, "SysT must be a derived System type");
		std::type_index typeI(typeid(SysT));
		for (auto & sys : mSystems) {
			if (sys.first == typeI) {
				NEO_LOG_E("Attempting to add a duplicate system %s", sys.second->mName.c_str());
				return static_cast<SysT &>(*sys.second);
			}
		}

		mSystems.push_back({ typeI, std::make_unique<SysT>(std::forward<Args>(args)...) });
		return static_cast<SysT &>(*mSystems.back().second);
	}

	template <typename SysT> 
	bool ECS::isSystemEnabled() const {
		static_assert(std::is_base_of<System, SysT>::value, "SysT must be a System type");
		static_assert(!std::is_same<SysT, System>::value, "SysT must be a derived System type");
		std::type_index typeI(typeid(SysT));
		for (auto & sys : mSystems) {
			if (sys.first == typeI && sys.second->mActive) {
				return true;
			}
		}
		return false;
	}

	template<typename CompT>
	CompT* ECS::getComponent(Entity e) {
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

		return mRegistry.try_get<CompT>(e);
	}

	template<typename CompT>
	CompT *const ECS::cGetComponent(Entity e) const {
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

		return const_cast<CompT *const>(mRegistry.try_get<CompT>(e));
	}

	template<typename CompT, typename... Args>
	CompT* ECS::addComponent(Entity e, Args &&... args) {
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

		CompT* component;
		if constexpr (sizeof...(Args) > 0) {
			component = new CompT(std::forward<Args>(args)...);
		}
		else {
			component = new CompT();
		}

		MM::EntityEditor<Entity>::ComponentInfo info;
		info.name = component->getName();
		info.create = [this](entt::registry& r, Entity e) {
			NEO_UNUSED(r, e);
			// addComponent<CompT>(e);
			NEO_LOG_W("Component creation unsupported");
		};
		info.destroy = [this](entt::registry& r, Entity e) {
			NEO_UNUSED(r);
			removeComponent<CompT>(e);
		};
		info.widget = [this](entt::registry& r, Entity e) {
			r.get<CompT>(e).imGuiEditor();
		};
		mEditor.registerComponent<CompT>(info);

		mAddComponentFuncs.push_back([e, component](Registry& registry) mutable {
			if (registry.try_get<CompT>(e)) {
				NEO_LOG_E("Attempting to add a second %s to entity %d when one already exists", component->getName().c_str(), e);
			}
			else {
				registry.emplace<CompT>(e, *component);
			}

			delete component;
			component = nullptr;
		});

		return component;
	}

	template<typename CompT>
	void ECS::removeComponent(Entity e) {
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");
		mRemoveComponentFuncs.push_back([e](Registry& registry) mutable {
			registry.remove<CompT>(e);
			});
	}

	template<typename... CompTs>
	auto ECS::getView() {
		TRACY_ZONE();
		return mRegistry.view<CompTs...>();
	}

	template<typename... CompTs>
	const auto ECS::getView() const {
		TRACY_ZONE();
		// TODO -- maybe force const inputs rather than attaching it
		// TODO -- otherwise view.get<> breaks
		return mRegistry.view<CompTs...>();
	}

	template<typename... CompTs>
	std::optional<std::tuple<ECS::Entity, CompTs&...>> ECS::getSingleView() {
		TRACY_ZONE();
		auto view = mRegistry.view<CompTs...>();
		if (view.size_hint() > 1) {
			NEO_LOG_E("Found %d entities when one was requested in %s", view.size_hint(), __FUNCSIG__);
		}
		if (view.size_hint()) {
			return { *view.each().begin() };
		}
		return std::nullopt;
	}

	// TODO -- maybe force const inputs rather than attaching it
	// TODO -- otherwise view.get<> breaks
	template<typename... CompTs>
	std::optional<std::tuple<ECS::Entity, const CompTs&...>> ECS::getSingleView() const {
		TRACY_ZONE();
		auto view = mRegistry.view<const CompTs...>();
		if (view.size_hint() > 1) {
			NEO_LOG_E("Found %d entities when one was requested in %s", view.size_hint(), __FUNCSIG__);
		}
		if (view.size_hint() == 1) {
			return { *view.each().begin() };
		}
		return std::nullopt;
	}

	template<typename... CompTs> 
	bool ECS::has() const {
		return mRegistry.view<CompTs...>().size_hint() != 0;
	}


	template<typename CompT> 
	void ECS::sort(std::function<bool(Entity left, Entity right)> compare) const {
		mRegistry.sort<CompT>(compare);
	}
}

