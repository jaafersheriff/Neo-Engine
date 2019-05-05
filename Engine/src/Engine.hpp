#pragma once

#include "Window/Window.hpp"
#include "Window/Mouse.hpp"
#include "Window/Keyboard.hpp"

#include "Loader/Library.hpp"
#include "Util/Util.hpp"

#include "Component/Components.hpp"
#include "Systems/Systems.hpp"

#include "ext/imgui/imgui.h"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <functional>

#include "GameObject/GameObject.hpp"

namespace neo {

    class Engine {

        /* Base Engine */
        public:
            static void init(const std::string &, const std::string &, int, const int);
            static void run();
            static void shutDown();

            static std::string APP_NAME;        /* Name of application */

        /* ECS */
        public:
            /* Create & destroy GameObjects */
            static GameObject & createGameObject();
            static void removeGameObject(GameObject &);

            /* Create a Component and attach it to a GameObject */
            template <typename CompT, typename... Args> static CompT & addComponent(GameObject *, Args &&...);
            /* Like addComponent by register component as SuperType */
            template <typename CompT, typename SuperType, typename... Args> static CompT & addComponentAs(GameObject *, Args &&...);

            /* Remove the component from the engine and its game object */
            template <typename CompT> static void removeComponent(CompT &);

            /* Attach a system */
            template <typename SysT, typename... Args> static SysT & addSystem(Args &&...);
            static void initSystems();

            /* Getters */
            static const std::vector<GameObject *> & getGameObjects() { return reinterpret_cast<const std::vector<GameObject *> &>(mGameObjects); }
            template <typename SysT> static SysT & getSystem();
            static const std::vector<std::pair<std::type_index, System *>> & getSystems() { return reinterpret_cast<const std::vector<std::pair<std::type_index, System *>> &>(mSystems); }
            template <typename CompT> static const std::vector<CompT *> & getComponents();
            template <typename CompT> static CompT* getSingleComponent();

            /* ImGui */
            static bool imGuiEnabled;
            static void toggleImGui() { imGuiEnabled = !imGuiEnabled; }
            static std::unordered_map<std::string, std::function<void()>> imGuiFuncs;
            static void addImGuiFunc(std::string name, std::function<void()> func) { imGuiFuncs.insert({ name, func}); }
            static void addDefaultImGuiFunc();

        private:
            /* Initialize / kill queues */
            static std::vector<std::unique_ptr<GameObject>> mGameObjectInitQueue;
            static std::vector<std::pair<std::type_index, std::unique_ptr<Component>>> mComponentInitQueue;
            static void _processInitQueue();
            static void _initGameObjects();
            static void _initComponents();
            static std::vector<GameObject *> mGameObjectKillQueue;
            static std::vector<std::pair<std::type_index, Component *>> mComponentKillQueue;
            static void _processKillQueue();
            static void _killGameObjects();
            static void _killComponents();

            /* Active containers */
            static std::vector<std::unique_ptr<GameObject>> mGameObjects;
            static std::unordered_map<std::type_index, std::unique_ptr<std::vector<std::unique_ptr<Component>>>> mComponents;
            static std::vector<std::pair<std::type_index, std::unique_ptr<System>>> mSystems;
    };

    /* Template implementation */
    template <typename CompT, typename... Args>
    CompT & Engine::addComponent(GameObject * gameObject, Args &&... args) {
        return addComponentAs<CompT, CompT, Args...>(gameObject, std::forward<Args>(args)...);
    }

    template <typename CompT, typename SuperT, typename... Args>
    CompT & Engine::addComponentAs(GameObject * gameObject, Args &&... args) {
        static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
        static_assert(std::is_base_of<SuperT, CompT>::value, "CompT must be derived from SuperT");
        static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

        mComponentInitQueue.emplace_back(typeid(SuperT), std::make_unique<CompT>(CompT(gameObject, std::forward<Args>(args)...)));
        return static_cast<CompT &>(*mComponentInitQueue.back().second);
    }

    template <typename SysT, typename... Args> 
    SysT & Engine::addSystem(Args &&... args) {
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

    template <typename SysT> 
    SysT & Engine::getSystem(void) {
        static_assert(std::is_base_of<System, SysT>::value, "SysT must be a System type");
        static_assert(!std::is_same<SysT, System>::value, "SysT must be a derived System type");

        std::type_index typeI(typeid(SysT));
        for (auto & sys : mSystems) {
            if (sys.first == typeI) {
                // this is valid because unique_ptr<T> is exactly the same data as T *
                return reinterpret_cast<SysT &>(*sys.second);
            }
        }

        assert(false);
    }

    template <typename CompT>
    void Engine::removeComponent(CompT& component) {
        if (!&component) {
            return;
        }
        static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
        static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

        assert(mComponents.count(typeid(CompT))); // trying to remove a type of component that was never added
        mComponentKillQueue.emplace_back(typeid(CompT), static_cast<Component *>(&component));
    }

    template <typename CompT>
    const std::vector<CompT *> & Engine::getComponents() {
        static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
        static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

        std::type_index typeI(typeid(CompT));
        auto it(mComponents.find(typeI));
        if (it == mComponents.end()) {
            mComponents.emplace(typeI, std::make_unique<std::vector<std::unique_ptr<Component>>>());
            it = mComponents.find(typeI);
        }
        // this is valid because unique_ptr<T> is exactly the same data as T *
        return reinterpret_cast<const std::vector<CompT *> &>(*(it->second));
    }

    template <typename CompT>
    CompT* Engine::getSingleComponent() {
        auto components = getComponents<CompT>();
        if (!components.size()) {
            return nullptr;
        }

        return components[0];
    }
}