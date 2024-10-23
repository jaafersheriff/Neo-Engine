#pragma once

#include "ECS/Systems/System.hpp"

#include "Util/Profiler.hpp"
#include "Util/Util.hpp"

#ifndef ENTT_ASSERT
#define ENTT_ASSERT(condition, ...) NEO_ASSERT(condition, __VA_ARGS__)
#endif
#include <entt/entt.hpp>

#include <ext/imgui_incl.hpp>
#pragma warning( push )
#pragma warning( disable : 4244 )
#define MM_IEEE_ASSERT(x) NEO_UNUSED(x)
#include <imgui_entt_entity_editor.hpp>
#pragma warning( pop )

#include <typeindex>
#include <optional>

namespace neo {
	class System;
	class Engine;

	class ECS {
		friend Engine;

	public:
		using Entity = entt::entity;
		using Registry = entt::registry;
		class EntityBuilder {
			friend ECS;
		public:
			EntityBuilder() = default;
			EntityBuilder(const EntityBuilder& other) {
				this->mComponents = other.mComponents;
			}

			template<typename CompT, typename... Args>
			EntityBuilder& attachComponent(Args &&... args) {
				// Holy moly wtf c++
				mComponents.emplace_back([args = std::make_tuple(std::forward<Args>(args) ...)](ECS& ecs, ECS::Entity e) mutable {
					std::apply([&ecs, e](auto&& ... args) {
						ecs.addComponent<CompT>(e, std::forward<decltype(args)>(args) ...);
						}, std::move(args));
					});

				return *this;
			}
	
		private:
			using AttachFunc = std::function<void(ECS& ecs, ECS::Entity e)>;
			std::vector<AttachFunc> mComponents;
		};


		ECS() = default;
		~ECS() = default;
		ECS(const ECS&) = delete;
		ECS& operator=(const ECS&) = delete;

		void submitEntity(EntityBuilder&& builder);
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
		template<typename CompT> uint32_t entityCount() const;
		template<typename CompT> bool has() const;
		template<typename CompT, typename... CompTs> bool has() const;
		template<typename... CompTs> auto getView();
		template<typename... CompTs> const auto getView() const;
		template<typename... CompTs> std::optional<std::tuple<Entity, CompTs&...>> getSingleView();
		template<typename... CompTs> std::optional<std::tuple<Entity, const CompTs&...>> getSingleView() const;

		template<typename FilterCompT, typename SortCompT> void sort(std::function<bool(Entity left, Entity right)> compare) const;

		/* Attach a system */
		template <typename SysT, typename... Args> SysT& addSystem(Args &&...);
		template <typename SysT> bool isSystemEnabled() const;
		template <typename SysT> void setSystemActive(bool active);


	private:
		mutable Registry mRegistry;
		mutable MM::EntityEditor<Entity> mEditor;

		/* Active containers */
		std::mutex mEntityCreationMutex;
		std::vector<EntityBuilder> mEntityCreateQueue;

		std::mutex mEntityKillMutex;
		std::vector<Entity> mEntityKillQueue;

		using ComponentModFunc = std::function<void(Registry&)>;
		std::mutex mAddComponentMutex;
		std::vector<ComponentModFunc> mAddComponentFuncs;

		std::mutex mRemoveComponentMutex;
		std::vector<ComponentModFunc> mRemoveComponentFuncs;

		std::vector<std::pair<std::type_index, std::unique_ptr<System>>> mSystems;
		void _initSystems();
		void _updateSystems(const ResourceManagers& resourceManagers);


		void _flush();
		void _clean();
		void _imguiEdtor();
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

	template <typename SysT> 
	void ECS::setSystemActive(bool active) {
		static_assert(std::is_base_of<System, SysT>::value, "SysT must be a System type");
		static_assert(!std::is_same<SysT, System>::value, "SysT must be a derived System type");
		std::type_index typeI(typeid(SysT));
		for (auto & sys : mSystems) {
			if (sys.first == typeI) {
				sys.second->mActive = active;
			}
		}
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
		info.name = component->mName;
		info.create = [this](entt::registry& r, Entity e) {
			NEO_UNUSED(r, e);
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

		{
			std::lock_guard<std::mutex> lock(mAddComponentMutex);
			mAddComponentFuncs.push_back([e, component](Registry& registry) mutable {
				if (registry.try_get<CompT>(e)) {
					NEO_LOG_E("Attempting to add a second %s to entity %d when one already exists", component->mName, e);
				}
				else {
					registry.emplace<CompT>(e, *component);
				}

				delete component;
				component = nullptr;
			});
		}

		return component;
	}

	template<typename CompT>
	void ECS::removeComponent(Entity e) {
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");
		std::lock_guard<std::mutex> lock(mRemoveComponentMutex);
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
		if (view.begin() != view.end()) {
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
		if (view.begin() != view.end()) {
			return { *view.each().begin() };
		}
		return std::nullopt;
	}

	template<typename CompT> 
	uint32_t ECS::entityCount() const {
		return static_cast<uint32_t>(mRegistry.storage<CompT>().size());
	}

	template<typename CompT>
	bool ECS::has() const {
		return entityCount<CompT>() > 0;
	}

	template<typename CompT, typename... CompTs> 
	bool ECS::has() const {
		return has<CompT>() && has<CompTs...>();
	}

	template<typename FilterCompT, typename SortCompT> 
	void ECS::sort(std::function<bool(const Entity left, const Entity right)> compare) const {
		mRegistry.sort<SortCompT>(compare);
		// EnTT can only sort against a single component ;( and then FilterCompT will be sorted against SortCompT
		mRegistry.sort<FilterCompT, SortCompT>();
	}
}

