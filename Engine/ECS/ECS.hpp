#pragma once

#include "Util/Util.hpp"

#include "ECS/ComponentTuple.hpp"
#include "ECS/Components.hpp"
#include "ECS/Systems/Systems.hpp"

#include "microprofile.h"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <optional>
#include <thread>

#include "ECS/GameObject.hpp"

namespace neo {
    class ComponentTuple;

    class ECS {
        friend Engine;

        public:
            void init();
            void shutDown();

        /* ECS */
        public:
            /* Create & destroy GameObjects */
            GameObject & createGameObject();
            void removeGameObject(GameObject &);

            /* Create a Component and attach it to a GameObject */
            template <typename CompT, typename... Args> CompT& addComponent(GameObject *, Args &&...);
            /* Like addComponent by register component as SuperType */
            template <typename CompT, typename SuperType, typename... Args> CompT& addComponentAs(GameObject *, Args &&...);

            /* Remove the component from the engine and its game object */
            template <typename CompT> void removeComponent(CompT&);

            /* Attach a system */
            template <typename SysT, typename... Args> SysT& addSystem(Args &&...);

            /* Getters */
            const std::vector<const GameObject*>& getGameObjects() const { return reinterpret_cast<const std::vector<const GameObject*> &>(mGameObjects); }
            template <typename CompT> const std::vector<const CompT*>& getComponents() const;
            template <typename CompT> const CompT* getSingleComponent() const;
            template <typename CompT, typename... CompTs> const std::unique_ptr<ComponentTuple> getComponentTuple(GameObject& go) const;
            template <typename CompT, typename... CompTs> const std::unique_ptr<const ComponentTuple> getComponentTuple() const;
            template <typename CompT, typename... CompTs> const std::vector<std::unique_ptr<const ComponentTuple>> getComponentTuples() const;

        private:
            /* Initialize / kill queues */
            std::vector<std::unique_ptr<GameObject>> mGameObjectInitQueue;
            std::vector<std::pair<std::type_index, std::unique_ptr<Component>>> mComponentInitQueue;
            void _processInitQueue();
            void _initGameObjects();
            void _initComponents();
            void _initSystems();
            std::vector<GameObject *> mGameObjectKillQueue;
            std::vector<std::pair<std::type_index, Component *>> mComponentKillQueue;
            void _removeComponent(std::type_index type, Component*);
            void _processKillQueue();
            void _killGameObjects();
            void _killComponents();

            /* Active containers */
            std::vector<std::unique_ptr<GameObject>> mGameObjects;
            std::unordered_map<std::type_index, std::unique_ptr<std::vector<std::unique_ptr<Component>>>> mComponents;
            std::vector<std::pair<std::type_index, std::unique_ptr<System>>> mSystems;
            void _updateSystems();

            void _imguiEdtor();
    };

    /* Template implementation */
    template <typename CompT, typename... Args>
    CompT& ECS::addComponent(GameObject * gameObject, Args &&... args) {
        return addComponentAs<CompT, CompT, Args...>(gameObject, std::forward<Args>(args)...);
    }

    template <typename CompT, typename SuperT, typename... Args>
    CompT& ECS::addComponentAs(GameObject * gameObject, Args &&... args) {
        static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
        static_assert(std::is_base_of<SuperT, CompT>::value, "CompT must be derived from SuperT");
        static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

        mComponentInitQueue.emplace_back(typeid(SuperT), std::make_unique<CompT>(CompT(gameObject, std::forward<Args>(args)...)));
        return static_cast<CompT &>(*mComponentInitQueue.back().second);
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

    template <typename CompT>
    void ECS::removeComponent(CompT& component) {
        if (!&component) {
            return;
        }
        static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
        static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

        _removeComponent(typeid(CompT), static_cast<Component*>(&component));
    }

    template <typename CompT>
    const std::vector<const CompT*> & ECS::getComponents() const {
        MICROPROFILE_SCOPEI("Engine", "getComponents", MP_AUTO);
        static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
        static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

        std::type_index typeI(typeid(CompT));
        auto it(mComponents.find(typeI));
        if (it == mComponents.end()) {
            mComponents.emplace(typeI, std::make_unique<std::vector<std::unique_ptr<Component>>>());
            it = mComponents.find(typeI);
        }
        // this is valid because unique_ptr<T> is exactly the same data as T *
        return reinterpret_cast<const std::vector<const CompT *> &>(*(it->second));
    }

    template <typename CompT>
    const CompT* ECS::getSingleComponent() const {
        MICROPROFILE_SCOPEI("Engine", "getSingleComponents", MP_AUTO);
        auto components = getComponents<CompT>();
        if (!components.size()) {
            return nullptr;
        }
        assert(components.size() == 1);

        return components[0];
    }

    template <typename CompT, typename... CompTs>
    const std::unique_ptr<const ComponentTuple> ECS::getComponentTuple() const {
        MICROPROFILE_SCOPEI("Engine", "getComponentTuple", MP_AUTO);
        for (auto comp : getComponents<CompT>()) {
            if (auto tuple = getComponentTuple<CompT, CompTs...>(comp->getGameObject())) {
                return tuple;
            }
        }
        return nullptr;
    }
 
    template <typename CompT, typename... CompTs>
    const std::vector<std::unique_ptr<const ComponentTuple>> ECS::getComponentTuples() const {
        MICROPROFILE_SCOPEI("Engine", "getComponentTuple", MP_AUTO);
        std::vector<std::unique_ptr<ComponentTuple>> tuples;
        for (auto comp : getComponents<CompT>()) {
            if (auto tuple = getComponentTuple<CompT, CompTs...>(comp->getGameObject())) {
                tuples.push_back(std::move(tuple));
            }
        }

        return tuples;
    }

    template <typename CompT, typename... CompTs>
    const std::unique_ptr<ComponentTuple> ECS::getComponentTuple(GameObject& go) const {
        std::unique_ptr<ComponentTuple> tuple = std::make_unique<ComponentTuple>(go);
        tuple->_addComponent<CompT>();
        tuple->populate<CompTs...>();

        if (*tuple) {
            return tuple;
        }

        tuple.release();
        return nullptr;
    }


}