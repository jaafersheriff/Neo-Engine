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

		Entity createEntity();
		void removeEntity(Entity e);

		// Entity access
		template<typename CompT, typename... Args> CompT* addComponent(Entity e, Args &&... args);
		template<typename CompT> void removeComponent(Entity e);

		template<typename CompT> bool has(Entity e);
		template<typename CompT> CompT* getComponent(Entity e);
		template<typename CompT> CompT *const cGetComponent(Entity e) const;
		template<typename SuperT, typename CompT> SuperT* getComponentAs(Entity e);
		template<typename SuperT, typename CompT> const SuperT* cGetComponentAs(Entity e) const;

		// All access
		template<typename CompT> CompT* getComponent();
		template<typename CompT> CompT *const cGetComponent() const;
		template<typename... CompTs> auto getView();
		template<typename... CompTs> const auto getView() const;

		/* Attach a system */
		template <typename SysT, typename... Args> SysT& addSystem(Args &&...);

	private:
		Registry mRegistry;
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
	CompT* ECS::getComponent() {
		MICROPROFILE_SCOPEI("ECS", "getComponent", MP_AUTO);
		auto view = mRegistry.view<CompT>();
		if (view.size() > 1) {
			NEO_LOG_W("Trying to get a single %s when multiple exist", mRegistry.try_get<CompT>(view.front())->getName().c_str());
		}
		if (view.size()) {
			return mRegistry.try_get<CompT>(view.front());
		}
		return nullptr;
	}

	template<typename CompT>
	CompT *const ECS::cGetComponent() const {
		MICROPROFILE_SCOPEI("ECS", "cGetComponent", MP_AUTO);
		auto view = mRegistry.view<CompT>();
		if (view.size() > 1) {
			NEO_LOG_W("Trying to get a single %s when multiple exist", mRegistry.try_get<CompT>(view.front())->getName().c_str());
		}
		if (view.size()) {
			return const_cast<CompT *const>(mRegistry.try_get<CompT>(view.front()));
		}
		return nullptr;
	}

	template<typename CompT>
	bool ECS::has(ECS::Entity e) {
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
	CompT* ECS::addComponent(Entity e, Args &&... args) {
		MICROPROFILE_SCOPEI("ECS", "addComponent", MP_AUTO);
		CompT* component;
		if constexpr (sizeof...(Args) > 0) {
			component = new CompT(std::forward<Args>(args)...);
		}
		else {
			component = new CompT();
		}

		mAddComponentFuncs.push_back([e, component](Registry& registry) mutable {
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
	void ECS::removeComponent(Entity e) {
		MICROPROFILE_SCOPEI("ECS", "removeComponent", MP_AUTO);
		mRemoveComponentFuncs.push_back([e](Registry& registry) mutable {
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
		return mRegistry.view<CompTs...>();
	}

	// template<typename... CompTs>
	// std::vector<ComponentTuple<CompTs...>> ECS::getComponentTuples() {
	//	 MICROPROFILE_SCOPEI("ECS", "getComponentTuples", MP_AUTO);
	// 	std::vector<ComponentTuple<CompTs...>> ret;
	// 	mRegistry.each([this, &ret](ECS::Entity entity) {
	//		 if (auto tuple = getComponentTuple<CompTs...>(entity)) {
	//			 ret.push_back(tuple);
	//		 }
	// 		});
	// 	return ret;
	// }

	// template<typename... CompTs>
	// const std::vector<ComponentTuple<CompTs...>> ECS::getComponentTuples() const {
	//	 MICROPROFILE_SCOPEI("ECS", "getComponentTuples", MP_AUTO);
	// 	std::vector<ComponentTuple<CompTs...>> ret;
	// 	mRegistry.each([this, &ret](ECS::Entity entity) {
	//		 if (auto tuple = ComponentTuple<CompTs...>(entity, std::move(const_cast<Registry&>(mRegistry).try_get<CompTs...>(entity)))) {
	//			 ret.push_back(tuple);
	//		 }
	// 		});
	// 	return ret;
	// }

	// template<typename... CompTs>
	// ComponentTuple<CompTs...> ECS::getComponentTuple() {
	//	 MICROPROFILE_SCOPEI("ECS", "getComponentTuple", MP_AUTO);
	//	 auto tuples = getComponentTuples<CompTs...>();
	//	 NEO_ASSERT(tuples.size() <= 1, "B");
	//	 return tuples[0];
	// }

	// template<typename... CompTs> 
	// const ComponentTuple<CompTs...> ECS::cGetComponentTuple() const {
	//	 MICROPROFILE_SCOPEI("ECS", "cGetComponentTuple", MP_AUTO);
	//	 auto tuples = getComponentTuples<CompTs...>();
	//	 NEO_ASSERT(tuples.size() == 1, "");
	//	 return tuples[0];
	// }

	template<typename SuperT, typename CompT> SuperT* ECS::getComponentAs(Entity e) {
		MICROPROFILE_SCOPEI("ECS", "getComponentAs", MP_AUTO);
		CompT* comp = getComponent<CompT>(e);
		NEO_ASSERT(comp, "A");
		return dynamic_cast<SuperT*>(comp);
	}
	template<typename SuperT, typename CompT> const SuperT* ECS::cGetComponentAs(Entity e) const {
		MICROPROFILE_SCOPEI("ECS", "cGetComponentAs", MP_AUTO);
		CompT *const comp = cGetComponent<CompT>(e);
		NEO_ASSERT(comp, "B");
		return dynamic_cast<SuperT *const>(comp);
	}
}
