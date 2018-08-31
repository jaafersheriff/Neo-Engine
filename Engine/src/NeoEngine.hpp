#pragma once

#include "Loader/Loader.hpp"
#include "Window/Window.hpp"
#include "Window/Mouse.hpp"
#include "Window/Keyboard.hpp"
#include "Util/Util.hpp"

#include "Component/Components.hpp"
#include "System/System.hpp"    // Necessary because there are no common systems in the engine
#include "System/Systems.hpp"

#include "ext/imgui/imgui.h"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <functional>

#include "GameObject/GameObject.hpp"

namespace neo {

    class NeoEngine {

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
            static const std::vector<GameObject *> & getGameObjects() { return reinterpret_cast<const std::vector<GameObject *> &>(gameObjects); }
            template <typename SysT> static SysT & getSystem();
            static const std::unordered_map<std::type_index, System *> & getSystems() { return reinterpret_cast<const std::unordered_map<std::type_index, System *> &>(systems); }
            template <typename CompT> static const std::vector<CompT *> & getComponents();

            /* ImGui */
            static bool imGuiEnabled;
            static void toggleImGui() { imGuiEnabled = !imGuiEnabled; }
            static std::unordered_map<std::string, std::function<void()>> imGuiFuncs;
            static void addImGuiFunc(std::string name, std::function<void()> func) { imGuiFuncs.insert({ name, func}); }

        private:
            /* Initialize / kill queues */
            static std::vector<std::unique_ptr<GameObject>> gameObjectInitQueue;
            static std::vector<std::pair<std::type_index, std::unique_ptr<Component>>> componentInitQueue;
            static void processInitQueue();
            static void initGameObjects();
            static void initComponents();
            static std::vector<GameObject *> gameObjectKillQueue;
            static std::vector<std::pair<std::type_index, Component *>> componentKillQueue;
            static void processKillQueue();
            static void killGameObjects();
            static void killComponents();

            /* Active containers */
            static std::vector<std::unique_ptr<GameObject>> gameObjects;
            static std::unordered_map<std::type_index, std::unique_ptr<std::vector<std::unique_ptr<Component>>>> components;
            static std::unordered_map<std::type_index, std::unique_ptr<System>> systems;


    };

    /* Template implementation */
    template <typename CompT, typename... Args>
    CompT & NeoEngine::addComponent(GameObject * gameObject, Args &&... args) {
        return addComponentAs<CompT, CompT, Args...>(gameObject, std::forward<Args>(args)...);
    }

    template <typename CompT, typename SuperT, typename... Args>
    CompT & NeoEngine::addComponentAs(GameObject * gameObject, Args &&... args) {
        static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
        static_assert(std::is_base_of<SuperT, CompT>::value, "CompT must be derived from SuperT");
        static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

        componentInitQueue.emplace_back(typeid(SuperT), std::make_unique<CompT>(CompT(gameObject, std::forward<Args>(args)...)));
        return static_cast<CompT &>(*componentInitQueue.back().second);
    }

    template <typename SysT, typename... Args> 
    SysT & NeoEngine::addSystem(Args &&... args) {
        static_assert(!std::is_same<SysT, System>::value, "SysT must be a derived system type");
        static_assert(!std::is_same<System, SysT>::value, "SysT must be a derived system type");
        std::type_index typeI(typeid(SysT));
        auto it(systems.find(typeI));
        if (it == systems.end()) {
            systems.emplace(typeI, std::make_unique<SysT>(std::forward<Args>(args)...));
            it = systems.find(typeI);
        }
 
        return static_cast<SysT &>(*systems.find(typeI)->second);
    }

    template <typename SysT> 
    SysT & NeoEngine::getSystem(void) {
        static_assert(!std::is_same<SysT, System>::value, "SysT must be a derived system type");
        static_assert(!std::is_same<System, SysT>::value, "SysT must be a derived system type");

        std::type_index typeI(typeid(SysT));
        assert(systems.count(typeI));
        // this is valid because unique_ptr<T> is exactly the same data as T *
        return reinterpret_cast<SysT &>(*(systems.find(typeI)->second));
    }

    template <typename CompT>
    void NeoEngine::removeComponent(CompT & component) {
        if (!&component) {
            return;
        }
        static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
        static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

        assert(components.count(typeid(CompT))); // trying to remove a type of component that was never added
        componentKillQueue.emplace_back(typeid(CompT), static_cast<Component *>(&component));
    }

    template <typename CompT>
    const std::vector<CompT *> & NeoEngine::getComponents() {
        static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
        static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

        std::type_index typeI(typeid(CompT));
        auto it(components.find(typeI));
        if (it == components.end()) {
            components.emplace(typeI, std::make_unique<std::vector<std::unique_ptr<Component>>>());
            it = components.find(typeI);
        }
        // this is valid because unique_ptr<T> is exactly the same data as T *
        return reinterpret_cast<const std::vector<CompT *> &>(*(it->second));
    }
}