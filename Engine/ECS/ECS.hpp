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

#include "ECS/GameObject.hpp"

#include "microprofile.h"

namespace neo {
    class ComponentTuple;
    class System;

    class ECS {
        friend Engine;

        public:
            ECS() = default;
            ~ECS() = default;
            ECS(const ECS&) = delete;
            ECS& operator=(const ECS&) = delete;

            void clean();

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
            template <typename CompT> const std::vector<CompT const*>& getComponents() const;
            template <typename CompT> const std::vector<CompT*>& getComponents();
            template <typename CompT> CompT const* getSingleComponent() const;
            template <typename CompT> CompT* getSingleComponent();
            template <typename CompT, typename... CompTs> std::unique_ptr<const ComponentTuple> getComponentTuple(const GameObject& go) const;
            template <typename CompT, typename... CompTs> std::unique_ptr<ComponentTuple> getComponentTuple(const GameObject& go);
            template <typename CompT, typename... CompTs> std::unique_ptr<const ComponentTuple> getComponentTuple() const;
            template <typename CompT, typename... CompTs> std::unique_ptr<ComponentTuple> getComponentTuple();
            template <typename CompT, typename... CompTs> const std::vector<std::unique_ptr<const ComponentTuple>> getComponentTuples() const;
            template <typename CompT, typename... CompTs> const std::vector<std::unique_ptr<ComponentTuple>> getComponentTuples();

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
            mutable std::unordered_map<std::type_index, std::unique_ptr<std::vector<std::unique_ptr<Component>>>> mComponents;
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
    const std::vector<CompT const*> & ECS::getComponents() const {
        MICROPROFILE_SCOPEI("Engine", "getComponents const", MP_AUTO);
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

    template <typename CompT> const std::vector<CompT*>& ECS::getComponents() {
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
        return reinterpret_cast<const std::vector<CompT*> &>(*(it->second));
    }

    template <typename CompT>
    CompT const* ECS::getSingleComponent() const {
        MICROPROFILE_SCOPEI("Engine", "getSingleComponents const", MP_AUTO);
        auto components = getComponents<CompT>();
        if (!components.size()) {
            return nullptr;
        }
        NEO_ASSERT(components.size() == 1, "Attempting to get a single component when there are many");

        return components[0];
    }

    template <typename CompT>
    CompT* ECS::getSingleComponent() {
        MICROPROFILE_SCOPEI("Engine", "getSingleComponents", MP_AUTO);
        auto components = getComponents<CompT>();
        if (!components.size()) {
            return nullptr;
        }
        NEO_ASSERT(components.size() == 1, "Attempting to get a single component when there are many");

        return components[0];
    }

    template <typename CompT, typename... CompTs>
    std::unique_ptr<const ComponentTuple> ECS::getComponentTuple() const {
        MICROPROFILE_SCOPEI("Engine", "getComponentTuple const", MP_AUTO);
        for (CompT const* comp : getComponents<CompT>()) {
            if (auto tuple = getComponentTuple<CompT, CompTs...>(comp->getGameObject())) {
                return tuple;
            }
        }
        return nullptr;
    }

    template <typename CompT, typename... CompTs>
    std::unique_ptr<ComponentTuple> ECS::getComponentTuple() {
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
        MICROPROFILE_SCOPEI("Engine", "getComponentTuples const", MP_AUTO);
        std::vector<std::unique_ptr<const ComponentTuple>> tuples;
        for (CompT const* comp : getComponents<CompT>()) {
            if (std::unique_ptr<const ComponentTuple> tuple = getComponentTuple<CompT, CompTs...>(comp->getGameObject())) {
                tuples.push_back(std::move(tuple));
            }
        }

        return tuples;
    }

    template <typename CompT, typename... CompTs>
    const std::vector<std::unique_ptr<ComponentTuple>> ECS::getComponentTuples() {
        MICROPROFILE_SCOPEI("Engine", "getComponentTuples", MP_AUTO);
        std::vector<std::unique_ptr<ComponentTuple>> tuples;
        for (CompT* comp : getComponents<CompT>()) {
            if (std::unique_ptr<ComponentTuple> tuple = getComponentTuple<CompT, CompTs...>(comp->getGameObject())) {
                tuples.push_back(std::move(tuple));
            }
        }

        return tuples;
    }

    template <typename CompT, typename... CompTs>
    std::unique_ptr<const ComponentTuple> ECS::getComponentTuple(const GameObject& go) const {
        MICROPROFILE_SCOPEI("Engine", "getComponentTuple(GameObject) const", MP_AUTO);
        std::unique_ptr<ComponentTuple> tuple = std::make_unique<ComponentTuple>(go);
        tuple->populate<CompT, CompTs...>();

        if (*tuple) {
            return tuple;
        }

        tuple.release();
        return nullptr;
    }

    template <typename CompT, typename... CompTs>
    std::unique_ptr<ComponentTuple> ECS::getComponentTuple(const GameObject& go) {
        MICROPROFILE_SCOPEI("Engine", "getComponentTuple(GameObject)", MP_AUTO);
        std::unique_ptr<ComponentTuple> tuple = std::make_unique<ComponentTuple>(go);
        tuple->populate<CompT, CompTs...>();

        if (*tuple) {
            return tuple;
        }

        tuple.release();
        return nullptr;
    }

}