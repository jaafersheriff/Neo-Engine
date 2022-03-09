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

        Entity createEntity();
        void removeEntity(Entity e);

        // Entity access
        template<typename CompT, typename... Args> CompT* addComponent(Entity e, Args... args);
        template<typename CompT> void removeComponent(Entity e);

        template<typename CompT> bool has(Entity e);
        template<typename CompT> CompT* getComponent(Entity e);
        template<typename CompT> const CompT* getComponent(Entity e) const;
        template<typename... CompTs> ComponentTuple<CompTs...> getComponentTuple(Entity e);
        template<typename... CompTs> const ComponentTuple<CompTs...>& cGetComponentTuple(const Entity e) const;

        // All access
        template<typename CompT> CompT* getComponent();
        template<typename... CompTs> View<CompTs...> getView();
        template<typename... CompTs> const View<CompTs...> getView() const;
        template<typename... CompTs> ComponentTuple<CompTs...> getComponentTuple(); // TODO - const
        template<typename... CompTs> std::vector<ComponentTuple<CompTs...>> getComponentTuples();
        template<typename... CompTs> std::vector<const ComponentTuple<CompTs...>> getComponentTuples() const;

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
        auto view = mRegistry.view<CompT>();
        if (view.size() != 1) {
            NEO_LOG_W("Trying to get a single %s when multiple exist", mRegistry.get<CompT>(view.front())->getName().c_str());
        }
        if (view.size()) {
            return mRegistry.try_get<CompT>(view.front());
        }
        return nullptr;
    }

    template<typename CompT>
    bool ECS::has(ECS::Entity e) {
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

	template<typename... CompTs>
	ComponentTuple<CompTs...> ECS::getComponentTuple(Entity e) {
        auto t = mRegistry.try_get<CompTs...>(e);
		return ComponentTuple<CompTs...>(e, t);
	}

	template<typename... CompTs>
	const ComponentTuple<CompTs...>& ECS::cGetComponentTuple(const Entity e) const {
		return ComponentTuple<CompTs...>(e, std::move(const_cast<Registry&>(mRegistry).try_get<CompTs...>(e)));
	}

	template<typename CompT>
	CompT* ECS::getComponent(Entity e) {
        return mRegistry.try_get<CompT>(e);
	}

	template<typename CompT>
	const CompT* ECS::getComponent(Entity e) const {
        return mRegistry.try_get<CompT>(e);
	}

	template<typename CompT, typename... Args>
	CompT* ECS::addComponent(Entity e, Args... args) {
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

		return component;
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
	std::vector<ComponentTuple<CompTs...>> ECS::getComponentTuples() {
		std::vector<ComponentTuple<CompTs...>> ret;
		mRegistry.each([this, &ret](ECS::Entity entity) {
			ret.push_back(getComponentTuple<CompTs...>(entity));
			});
		return ret;
	}

	template<typename... CompTs>
	std::vector<const ComponentTuple<CompTs...>> ECS::getComponentTuples() const {
		std::vector<ComponentTuple<CompTs...>> ret;
		mRegistry.each([this, &ret](ECS::Entity entity) {
			ret.push_back(cGetComponentTuple<CompTs...>(entity));
			});
		return ret;
	}

	template<typename... CompTs>
	ComponentTuple<CompTs...> ECS::getComponentTuple() {
        auto tuples = getComponentsTuples<CompTs...>();
        NEO_ASSERT(tuples.size() == 1, "");
        return tuples[0];
	}
}
