#pragma once

#include "Util/Util.hpp"

#include "ECS/ComponentTuple.hpp"
#include "ECS/Systems/System.hpp"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <optional>
#include <thread>

#include <microprofile.h>
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

				template<typename... CompTs>
				using View = entt::basic_view<Entity, entt::get_t<CompTs...>, entt::exclude_t<>>;

            /* Attach a system */
            template <typename SysT, typename... Args> SysT& addSystem(Args &&...);

            /* Old Getters */
            // const std::vector<GameObject*>& getGameObjects() const { return reinterpret_cast<const std::vector<GameObject*>&>(mGameObjects); }
            // const std::vector<const GameObject*>& cGetGameObjects() const { return reinterpret_cast<const std::vector<const GameObject*> &>(mGameObjects); }
            // template <typename CompT> const std::vector<CompT const*>& getComponents() const;
            // template <typename CompT> const std::vector<CompT*>& getComponents();
            // template <typename CompT> CompT const* getSingleComponent() const;
            // template <typename CompT> CompT* getSingleComponent();
            // template <typename CompT, typename... CompTs> std::unique_ptr<const ComponentTuple> getComponentTuple(const GameObject& go) const;
            // template <typename CompT, typename... CompTs> std::unique_ptr<ComponentTuple> getComponentTuple(const GameObject& go);
            // template <typename CompT, typename... CompTs> std::unique_ptr<const ComponentTuple> getComponentTuple() const;
            // template <typename CompT, typename... CompTs> std::unique_ptr<ComponentTuple> getComponentTuple();
            // template <typename CompT, typename... CompTs> const std::vector<std::unique_ptr<const ComponentTuple>> getComponentTuples() const;
            // template <typename CompT, typename... CompTs> const std::vector<std::unique_ptr<ComponentTuple>> getComponentTuples();

				Entity createEntity();
				void removeEntity(Entity e);

				// Entity access
				template<typename CompT, typename... Args> CompT& addComponent(Entity e, Args... args);
				template<typename CompT> void removeComponent(Entity e);

                template<typename CompT> bool has(Entity e);
				template<typename CompT> CompT& getComponent(Entity e);
				template<typename CompT> const CompT& getComponent(Entity e) const;
				template<typename... CompTs> ComponentTuple<CompTs...> getComponentTuple(const Entity e);
				template<typename... CompTs> const ComponentTuple<CompTs...>& cGetComponentTuple(const Entity e) const;

				// All access
                template<typename CompT> CompT& getComponent();
				template<typename... CompTs> View<CompTs...> getView();
				template<typename... CompTs> const View<CompTs...> getView() const;
				template<typename... CompTs> auto getSingleView();
				template<typename... CompTs> const auto getSingleView() const;
				template<typename... CompTs> std::vector<ComponentTuple<CompTs...>> getComponentTuples();
				template<typename... CompTs> std::vector<const ComponentTuple<CompTs...>> getComponentTuples() const;

				Registry mRegistry;
        private:
            /* Active containers */
				std::vector<Entity> mEntityKillQueue;
				using ComponentModFunc = std::function<void(Registry&)>;
				std::vector<ComponentModFunc> mAddComponentFuncs;
				std::vector<ComponentModFunc> mRemoveComponentFuncs;

            std::vector<std::pair<std::type_index, std::unique_ptr<System>>> mSystems;
            void _updateSystems();
    };

    template<typename CompT>
    CompT& ECS::getComponent() {
        auto view = mRegistry.view<FrameStatsComponent>();
        NEO_ASSERT(view.size() == 1, "wtf");
        return mRegistry.get<FrameStatsComponent>(view.front());
    }

    template<typename CompT>
    bool ECS::has(ECS::Entity e) {
        return mRegistry.has<FrameStatsComponent>(e);
    }

    /* Template implementation */
    // template <typename CompT, typename... Args>
    // CompT& ECS::addComponent(GameObject * gameObject, Args &&... args) {
    //     return addComponentAs<CompT, CompT, Args...>(gameObject, std::forward<Args>(args)...);
    // }

    // template <typename CompT, typename SuperT, typename... Args>
    // CompT& ECS::addComponentAs(GameObject * gameObject, Args &&... args) {
    //     static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
    //     static_assert(std::is_base_of<SuperT, CompT>::value, "CompT must be derived from SuperT");
    //     static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

    //     mComponentInitQueue.emplace_back(typeid(SuperT), std::make_unique<CompT>(CompT(gameObject, std::forward<Args>(args)...)));
    //     return static_cast<CompT &>(*mComponentInitQueue.back().second);
    // }

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

    // template <typename CompT>
    // void ECS::removeComponent(CompT& component) {
    //     if (!&component) {
    //         return;
    //     }
    //     static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
    //     static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

    //     _removeComponent(typeid(CompT), static_cast<Component*>(&component));
    // }

    // template <typename CompT>
    // const std::vector<CompT const*> & ECS::getComponents() const {
    //     MICROPROFILE_SCOPEI("Engine", "getComponents const", MP_AUTO);
    //     static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
    //     static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

    //     std::type_index typeI(typeid(CompT));
    //     auto it(mComponents.find(typeI));
    //     if (it == mComponents.end()) {
    //         mComponents.emplace(typeI, std::make_unique<std::vector<std::unique_ptr<Component>>>());
    //         it = mComponents.find(typeI);
    //     }
    //     // this is valid because unique_ptr<T> is exactly the same data as T *
    //     return reinterpret_cast<const std::vector<const CompT *> &>(*(it->second));
    // }

    // template <typename CompT> const std::vector<CompT*>& ECS::getComponents() {
    //     MICROPROFILE_SCOPEI("Engine", "getComponents", MP_AUTO);
    //     static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
    //     static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

    //     std::type_index typeI(typeid(CompT));
    //     auto it(mComponents.find(typeI));
    //     if (it == mComponents.end()) {
    //         mComponents.emplace(typeI, std::make_unique<std::vector<std::unique_ptr<Component>>>());
    //         it = mComponents.find(typeI);
    //     }
    //     // this is valid because unique_ptr<T> is exactly the same data as T *
    //     return reinterpret_cast<const std::vector<CompT*> &>(*(it->second));
    // }

    // template <typename CompT>
    // CompT const* ECS::getSingleComponent() const {
    //     MICROPROFILE_SCOPEI("Engine", "getSingleComponents const", MP_AUTO);
    //     auto components = getComponents<CompT>();
    //     if (!components.size()) {
    //         return nullptr;
    //     }
    //     NEO_ASSERT(components.size() == 1, "Attempting to get a single component when there are many");

    //     return components[0];
    // }

    // template <typename CompT>
    // CompT* ECS::getSingleComponent() {
    //     MICROPROFILE_SCOPEI("Engine", "getSingleComponents", MP_AUTO);
    //     auto components = getComponents<CompT>();
    //     if (!components.size()) {
    //         return nullptr;
    //     }
    //     NEO_ASSERT(components.size() == 1, "Attempting to get a single component when there are many");

    //     return components[0];
    // }

    // template <typename CompT, typename... CompTs>
    // std::unique_ptr<const ComponentTuple> ECS::getComponentTuple() const {
    //     MICROPROFILE_SCOPEI("Engine", "getComponentTuple const", MP_AUTO);
    //     for (CompT const* comp : getComponents<CompT>()) {
    //         if (auto tuple = getComponentTuple<CompT, CompTs...>(comp->getGameObject())) {
    //             return tuple;
    //         }
    //     }
    //     return nullptr;
    // }

    // template <typename CompT, typename... CompTs>
    // std::unique_ptr<ComponentTuple> ECS::getComponentTuple() {
    //     MICROPROFILE_SCOPEI("Engine", "getComponentTuple", MP_AUTO);
    //     for (auto comp : getComponents<CompT>()) {
    //         if (auto tuple = getComponentTuple<CompT, CompTs...>(comp->getGameObject())) {
    //             return tuple;
    //         }
    //     }
    //     return nullptr;
    // }

    // template <typename CompT, typename... CompTs>
    // const std::vector<std::unique_ptr<const ComponentTuple>> ECS::getComponentTuples() const {
    //     MICROPROFILE_SCOPEI("Engine", "getComponentTuples const", MP_AUTO);
    //     std::vector<std::unique_ptr<const ComponentTuple>> tuples;
    //     for (CompT const* comp : getComponents<CompT>()) {
    //         if (std::unique_ptr<const ComponentTuple> tuple = getComponentTuple<CompT, CompTs...>(comp->getGameObject())) {
    //             tuples.push_back(std::move(tuple));
    //         }
    //     }

    //     return tuples;
    // }

    // template <typename CompT, typename... CompTs>
    // const std::vector<std::unique_ptr<ComponentTuple>> ECS::getComponentTuples() {
    //     MICROPROFILE_SCOPEI("Engine", "getComponentTuples", MP_AUTO);
    //     std::vector<std::unique_ptr<ComponentTuple>> tuples;
    //     for (CompT* comp : getComponents<CompT>()) {
    //         if (std::unique_ptr<ComponentTuple> tuple = getComponentTuple<CompT, CompTs...>(comp->getGameObject())) {
    //             tuples.push_back(std::move(tuple));
    //         }
    //     }

    //     return tuples;
    // }

    // template <typename CompT, typename... CompTs>
    // std::unique_ptr<const ComponentTuple> ECS::getComponentTuple(const GameObject& go) const {
    //     MICROPROFILE_SCOPEI("Engine", "getComponentTuple(GameObject) const", MP_AUTO);
    //     std::unique_ptr<ComponentTuple> tuple = std::make_unique<ComponentTuple>(go);
    //     tuple->populate<CompT, CompTs...>();

    //     if (*tuple) {
    //         return tuple;
    //     }

    //     tuple.release();
    //     return nullptr;
    // }

    // template <typename CompT, typename... CompTs>
    // std::unique_ptr<ComponentTuple> ECS::getComponentTuple(const GameObject& go) {
    //     MICROPROFILE_SCOPEI("Engine", "getComponentTuple(GameObject)", MP_AUTO);
    //     std::unique_ptr<ComponentTuple> tuple = std::make_unique<ComponentTuple>(go);
    //     tuple->populate<CompT, CompTs...>();

    //     if (*tuple) {
    //         return tuple;
    //     }

    //     tuple.release();
    //     return nullptr;
    // }

	template<typename... CompTs>
	ComponentTuple<CompTs...> ECS::getComponentTuple(const Entity e) {
		return ComponentTuple<CompTs...>(std::move(mRegistry.try_get<CompTs...>(e)));
	}

	template<typename... CompTs>
	const ComponentTuple<CompTs...>& ECS::cGetComponentTuple(const Entity e) const {
		return ComponentTuple<CompTs...>(std::move(const_cast<Registry&>(mRegistry).try_get<CompTs...>(e)));
	}

	template<typename CompT>
	CompT& ECS::getComponent(Entity e) {
		if (auto comp = mRegistry.try_get<CompT>(e)) {
			return *comp;
		}
        NEO_FAIL("Err");
	}

	template<typename CompT>
	const CompT& ECS::getComponent(Entity e) const {
		if (auto comp = mRegistry.try_get<CompT>(e)) {
			return *comp;
		}
        NEO_FAIL("Err");
	}

	template<typename CompT, typename... Args>
	CompT& ECS::addComponent(Entity e, Args... args) {
		CompT* component;
		if constexpr (sizeof...(Args) > 0) {
			component = new CompT(std::forward<Args...>(args...));
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

		return *component;
	}

	template<typename CompT>
	void ECS::removeComponent(Entity e) {
		mRemoveComponentFuncs.push_back([e](Registry& registry) mutable {
			registry.remove<CompT>(e);
			});
	}

	template<typename... CompTs>
	ECS::View<CompTs...> ECS::getView() {
		return mRegistry.view<CompTs...>();
	}

	template<typename... CompTs>
	const ECS::View<CompTs...> ECS::getView() const {
		return mRegistry.view<CompTs...>();
	}

	template<typename... CompTs>
	auto ECS::getSingleView() {
		return *mRegistry.view<CompTs...>().begin();
	}

	template<typename... CompTs>
	const auto ECS::getSingleView() const {
		return *mRegistry.view<CompTs...>().begin();
	}

	template<typename... CompTs>
	std::vector<ComponentTuple<CompTs...>> ECS::getComponentTuples() {
		std::vector<ECS::ComponentTuple<CompTs...>> ret;
		mRegistry.each([this, &ret](auto entity) {
			ret.push_back(getComponentTuple<CompTs...>(entity));
			});
		return ret;
	}

	template<typename... CompTs>
	std::vector<const ComponentTuple<CompTs...>> ECS::getComponentTuples() const {
		std::vector<ECS::ComponentTuple<CompTs...>> ret;
		mRegistry.each([this, &ret](auto entity) {
			ret.push_back(cGetComponentTuple<CompTs...>(entity));
			});
		return ret;
	}
}
