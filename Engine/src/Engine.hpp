#pragma once

#include "Window/Window.hpp"
#include "Window/Mouse.hpp"
#include "Window/Keyboard.hpp"

#include "Renderer/Renderer.hpp"
#include "Loader/Library.hpp"
#include "Util/Util.hpp"

#include "ECS/ComponentTuple.hpp"
#include "ECS/Component/Components.hpp"
#include "ECS/Systems/Systems.hpp"

#include "ext/imgui/imgui.h"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <optional>

#include "ECS/GameObject.hpp"

#include "ext/microprofile.h"

namespace neo {
    class ComponentTuple;

    struct EngineConfig {
        std::string APP_NAME = "Neo Engine";
        std::string APP_RES = "";
        int width = 1920;
        int height = 1080;
        bool attachEditor = true;
    };

    class Engine {

        /* Base Engine */
        public:
            static EngineConfig mConfig;
            static void init(EngineConfig);
            static void run();
            static void shutDown();

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

            /* Getters */
            static const std::vector<GameObject *> & getGameObjects() { return reinterpret_cast<const std::vector<GameObject *> &>(mGameObjects); }
            template <typename SysT> static SysT & getSystem();
            static const std::vector<std::pair<std::type_index, System *>> & getSystems() { return reinterpret_cast<const std::vector<std::pair<std::type_index, System *>> &>(mSystems); }
            template <typename CompT> static const std::vector<CompT *> & getComponents();
            template <typename CompT> static CompT* getSingleComponent();
            template <typename CompT, typename... CompTs> static std::unique_ptr<ComponentTuple> getComponentTuple(GameObject& go);
            template <typename CompT, typename... CompTs> static std::unique_ptr<ComponentTuple> getComponentTuple();
            template <typename CompT, typename... CompTs> static std::vector<std::unique_ptr<ComponentTuple>> getComponentTuples();

            /* ImGui */
            static bool mImGuiEnabled;
            static void toggleImGui() { mImGuiEnabled = !mImGuiEnabled; }
            static void addImGuiFunc(std::string name, std::function<void()> func) { mImGuiFuncs.insert({ name, func}); }

        private:
            /* Initialize / kill queues */
            static std::vector<std::unique_ptr<GameObject>> mGameObjectInitQueue;
            static std::vector<std::pair<std::type_index, std::unique_ptr<Component>>> mComponentInitQueue;
            static void _processInitQueue();
            static void _initGameObjects();
            static void _initComponents();
            static void _initSystems();
            static std::vector<GameObject *> mGameObjectKillQueue;
            static std::vector<std::pair<std::type_index, Component *>> mComponentKillQueue;
            static void _removeComponent(std::type_index type, Component*);
            static void _processKillQueue();
            static void _killGameObjects();
            static void _killComponents();

            /* Active containers */
            static std::vector<std::unique_ptr<GameObject>> mGameObjects;
            static std::unordered_map<std::type_index, std::unique_ptr<std::vector<std::unique_ptr<Component>>>> mComponents;
            static std::vector<std::pair<std::type_index, std::unique_ptr<System>>> mSystems;

            /* ImGui */
            static std::unordered_map<std::string, std::function<void()>> mImGuiFuncs;
            static void _runImGui();
 
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

        _removeComponent(typeid(CompT), static_cast<Component*>(&component));
    }

    template <typename CompT>
    const std::vector<CompT *> & Engine::getComponents() {
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
        return reinterpret_cast<const std::vector<CompT *> &>(*(it->second));
    }

    template <typename CompT>
    CompT* Engine::getSingleComponent() {
        MICROPROFILE_SCOPEI("Engine", "getSingleComponents", MP_AUTO);
        auto components = getComponents<CompT>();
        if (!components.size()) {
            return nullptr;
        }
        assert(components.size() == 1);

        return components[0];
    }

    template <typename CompT, typename... CompTs>
    std::unique_ptr<ComponentTuple> Engine::getComponentTuple() {
        MICROPROFILE_SCOPEI("Engine", "getComponentTuple", MP_AUTO);
        for (auto comp : getComponents<CompT>()) {
            if (auto tuple = getComponentTuple<CompT, CompTs...>(comp->getGameObject())) {
                return tuple;
            }
        }
        return nullptr;
    }
 
    template <typename CompT, typename... CompTs>
    std::vector<std::unique_ptr<ComponentTuple>> Engine::getComponentTuples() {
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
    std::unique_ptr<ComponentTuple> Engine::getComponentTuple(GameObject& go) {
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