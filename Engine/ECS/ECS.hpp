#pragma once

#include "Util/Util.hpp"

#include "ECS/Systems/System.hpp"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <optional>
#include <thread>

#include <microprofile.h>

#ifndef ENTT_ASSERT
#define ENTT_ASSERT(condition, ...) NEO_ASSERT(condition, __VA_ARGS__)
#endif
#include <entt/entt.hpp>

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

		/* Const because these jobs are executed at the beginning of the name frame on flush() */
		Entity createEntity() const;
		void removeEntity(Entity e) const;
		template<typename CompT, typename... Args> CompT* addComponent(Entity e, Args &&... args) const;
		template<typename CompT> void removeComponent(Entity e) const;

		/* Entity access */
		template<typename CompT> bool has(Entity e) const;
		template<typename CompT> CompT* getComponent(Entity e);
		template<typename CompT> CompT *const cGetComponent(Entity e) const;
		template<typename SuperT, typename CompT, typename... CompTs> SuperT* getOneOfAs(Entity e);
		template<typename SuperT, typename CompT, typename... CompTs> const SuperT* getOneOfAs(Entity e) const;

		/* All access */
		template<typename... CompTs> bool has() const;
		template<typename CompT> std::optional<std::tuple<Entity, CompT&>> getComponent();
		template<typename CompT> std::optional<std::tuple<ECS::Entity, const CompT&>> cGetComponent() const;
		template<typename... CompTs> auto getView();
		template<typename... CompTs> const auto getView() const;
		template<typename... CompTs> std::optional<std::tuple<Entity, CompTs&...>> getSingleView();
		template<typename... CompTs> std::optional<std::tuple<Entity, const CompTs&...>> getSingleView() const;

		/* Attach a system */
		template <typename SysT, typename... Args> SysT& addSystem(Args &&...);

	private:
		mutable Registry mRegistry;
		/* Active containers */
		mutable std::vector<Entity> mEntityKillQueue;
		using ComponentModFunc = std::function<void(Registry&)>;
		std::vector<ComponentModFunc> mAddComponentFuncs;
		std::vector<ComponentModFunc> mRemoveComponentFuncs;

		std::vector<std::pair<std::type_index, std::unique_ptr<System>>> mSystems;
		void _initSystems();
		void _updateSystems();
	};

	template<typename CompT>
	std::optional<std::tuple<ECS::Entity, CompT&>> ECS::getComponent() {
		MICROPROFILE_SCOPEI("ECS", "getComponent", MP_AUTO);
		auto view = mRegistry.view<CompT>();
		NEO_ASSERT(view.size() <= 1, "");
		if (view.size() == 1) {
			return { *view.each().begin() };
		}
		return std::nullopt;
	}

	template<typename CompT>
	std::optional<std::tuple<ECS::Entity, const CompT&>> ECS::cGetComponent() const {
		MICROPROFILE_SCOPEI("ECS", "cGetComponent", MP_AUTO);
		auto view = mRegistry.view<CompT>();
		if (view.size() > 1) {
			NEO_LOG_W("Trying to get a single %s when multiple exist", mRegistry.try_get<CompT>(view.front())->getName().c_str());
		}
		if (view.size()) {
			return { *view.each().begin() };
		}
		return std::nullopt;
	}

	template<typename CompT>
	bool ECS::has(ECS::Entity e) const {
		MICROPROFILE_SCOPEI("ECS", "has", MP_AUTO);
		return mRegistry.try_get<CompT>(e) != nullptr;
	}

	template <typename SysT, typename... Args> 
	SysT& ECS::addSystem(Args &&... args) {
		static_assert(std::is_base_of<System, SysT>::value, "SysT must be a System type");
		static_assert(!std::is_same<SysT, System>::value, "SysT must be a derived System type");
		std::type_index typeI(typeid(SysT));
		for (auto & sys : mSystems) {
			if (sys.first == typeI) {
				return static_cast<SysT &>(*sys.second);
			}
		}

		mSystems.push_back({ typeI, std::make_unique<SysT>(std::forward<Args>(args)...) });
		return static_cast<SysT &>(*mSystems.back().second);
	}

	template<typename CompT>
	CompT* ECS::getComponent(Entity e) {
		MICROPROFILE_SCOPEI("ECS", "getComponent", MP_AUTO);
		return mRegistry.try_get<CompT>(e);
	}

	template<typename CompT>
	CompT *const ECS::cGetComponent(Entity e) const {
		MICROPROFILE_SCOPEI("ECS", "cGetComponent", MP_AUTO);
		return const_cast<CompT *const>(mRegistry.try_get<CompT>(e));
	}

	template<typename CompT, typename... Args>
	CompT* ECS::addComponent(Entity e, Args &&... args) const {
		MICROPROFILE_SCOPEI("ECS", "addComponent", MP_AUTO);
		CompT* component;
		if constexpr (sizeof...(Args) > 0) {
			component = new CompT(std::forward<Args>(args)...);
		}
		else {
			component = new CompT();
		}

		mAddComponentFuncs.push_back([e, component](Registry& registry) mutable {
			MICROPROFILE_SCOPEI("ECS", "lambda addComponent", MP_AUTO);
			if (registry.try_get<CompT>(e)) {
				NEO_FAIL("Err");
			}

			registry.emplace<CompT>(e, *component);
			delete component;
			component = nullptr;
			});

		return component;
	}

	template<typename CompT>
	void ECS::removeComponent(Entity e) const {
		MICROPROFILE_SCOPEI("ECS", "removeComponent", MP_AUTO);
		mRemoveComponentFuncs.push_back([e](Registry& registry) mutable {
			MICROPROFILE_SCOPEI("ECS", "lambda removeComponent", MP_AUTO);
			registry.remove<CompT>(e);
			});
	}

	template<typename... CompTs>
	auto ECS::getView() {
		MICROPROFILE_SCOPEI("ECS", "getView", MP_AUTO);
		return mRegistry.view<CompTs...>();
	}

	template<typename... CompTs>
	const auto ECS::getView() const {
		MICROPROFILE_SCOPEI("ECS", "getView", MP_AUTO);
		// TODO -- maybe force const inputs rather than attaching it
		// TODO -- otherwise view.get<> breaks
		return mRegistry.view<const CompTs...>();
	}

	template<typename SuperT, typename CompT, typename... CompTs> SuperT* ECS::getOneOfAs(Entity e) {
		MICROPROFILE_SCOPEI("ECS", "getOneOfAs", MP_AUTO);
		static_assert(std::is_base_of<SuperT, CompT>::value, "TODO");
		if (auto comp = mRegistry.try_get<CompT>(e)) {
			return dynamic_cast<SuperT*>(comp);
		}
		if constexpr (sizeof...(CompTs) > 0) {
			return getOneOfAs<SuperT, CompTs...>(e);
		}
		else {
			// assert?
			return nullptr;
		}
	}

	template<typename SuperT, typename CompT, typename... CompTs> const SuperT * ECS::getOneOfAs(Entity e) const {
		MICROPROFILE_SCOPEI("ECS", "getOneOfAs", MP_AUTO);
		static_assert(std::is_base_of<SuperT, CompT>::value, "TODO");
		if (auto comp = mRegistry.try_get<CompT>(e)) {
			return dynamic_cast<const SuperT *>(comp);
		}
		if constexpr (sizeof...(CompTs) > 0) {
			return getOneOfAs<SuperT, CompTs...>(e);
		}
		else {
			// assert?
			return nullptr;
		}
	}

	template<typename... CompTs> std::optional<std::tuple<ECS::Entity, CompTs&...>> ECS::getSingleView() {
		// TODO - assert that theres more than one compt
		MICROPROFILE_SCOPEI("ECS", "getSingleView", MP_AUTO);
		auto view = mRegistry.view<CompTs...>();
		NEO_ASSERT(view.size_hint() <= 1, "Found %d entities when one was requested", view.size_hint());
		for (auto entity : view) {
			if (view.contains(entity)) {
				return std::tuple_cat(std::make_tuple(entity), view.get<CompTs...>(entity));
			}
		}
		return std::nullopt;
	}

	// TODO -- maybe force const inputs rather than attaching it
	// TODO -- otherwise view.get<> breaks
	template<typename... CompTs> std::optional<std::tuple<ECS::Entity, const CompTs&...>> ECS::getSingleView() const {
		MICROPROFILE_SCOPEI("ECS", "getSingleView", MP_AUTO);
		auto view = mRegistry.view<const CompTs...>();
		// NEO_ASSERT(view.size_hint() <= 1, "Found %d entities when one was requested", view.size_hint());
		for (auto entity : view) {
			if (view.contains(entity)) {
				return std::tuple_cat(std::make_tuple(entity), view.get<const CompTs...>(entity));
			}
		}
		return std::nullopt;
	}

	template<typename... CompTs> bool ECS::has() const {
		MICROPROFILE_SCOPEI("ECS", "has", MP_AUTO);
		return mRegistry.view<CompTs...>().size_hint() != 0;
	}
}
