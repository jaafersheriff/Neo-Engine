#pragma once

#include "Window/Window.hpp"
#include "GameObject/GameObject.hpp"
#include "Component/Component.hpp"
#include "System/System.hpp"

#include "Component/Components.hpp"
#include "System/Systems.hpp"

#include "ext/imgui/imgui.h"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <functional>

namespace neo {

    class NeoEngine {

        /* Base Engine */
        public:
            static void init(const std::string &, const std::string &, int, const int);
            static void run();
            static void shutDown();

            static std::string ENGINE_RES_DIR;  /* Engine's resource directory */
            static std::string APP_RES_DIR;     /* App's resource directory */
            static std::string APP_NAME;        /* Name of application */

        /* ECS */
        public:
            /* Create & destroy GameObjects */
            static GameObject & createGameObject();
            static void killGameObject(GameObject &);

            /* Create a Component and attach it to a GameObject */
            template <typename CompT, typename... Args> static CompT & addComponent(GameObject &, Args &&...);
            /* Like addComponent by register component as SuperType */
            template <typename CompT, typename SuperType, typename... Args> static CompT & addComponentAs(GameObject &, Args &&...);

            /* Remove the component from the engine and its game object */
            template <typename CompT> static void removeComponent(CompT &);

            /* Attach a system */
            template <typename SysT, typename... Args> static SysT & addSystem(Args &&...);
            static void initSystems();

            /* Getters */
            static const std::vector<GameObject *> & getGameObjects() { return reinterpret_cast<const std::vector<GameObject *> &>(gameObjects); }
            static const std::vector<System *> & getSystems() { return reinterpret_cast<const std::vector<System *> &>(systems); }
            template <typename CompT> static const std::vector<CompT *> & getComponents();

            /* ImGui */
            static bool imGuiEnabled;
            static void toggleImGui() { imGuiEnabled = !imGuiEnabled; }
            static std::vector<std::function<void()>> imGuiFuncs;
            static void addImGuiFunc(std::function<void()> func) { imGuiFuncs.push_back(func); }

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
            static std::vector<std::unique_ptr<System>> systems;

        /* FPS*/
        public:
            static int FPS;                 /* Frames per second */
            static double timeStep;         /* Delta time */
        private:
            static void updateFPS();
            static double lastFPSTime;      /* Time at which last FPS was calculated */
            static int nFrames;             /* Number of frames in current second */
            static double lastFrameTime;    /* Time at which last frame was rendered */
            static double runTime;          /* Global timer */
    };

    /* Template implementation */
    template <typename CompT, typename... Args>
    CompT & NeoEngine::addComponent(GameObject & gameObject, Args &&... args) {
        return addComponentAs<CompT, CompT, Args...>(gameObject, std::forward<Args>(args)...);
    }

    template <typename CompT, typename SuperT, typename... Args>
    CompT & NeoEngine::addComponentAs(GameObject & gameObject, Args &&... args) {
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
        systems.emplace_back(std::make_unique<SysT>(std::forward<Args>(args)...));
        return static_cast<SysT &>(*systems.back());
    }

    template <typename CompT>
    void NeoEngine::removeComponent(CompT & component) {
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