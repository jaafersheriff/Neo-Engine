#include "NeoEngine.hpp"

#include <time.h>
#include <iostream>

namespace neo {

    /* Base Engine */
    std::string NeoEngine::APP_NAME = "Neo Engine";

    /* ECS */
    std::vector<std::unique_ptr<GameObject>> NeoEngine::gameObjects;
    std::unordered_map<std::type_index, std::unique_ptr<std::vector<std::unique_ptr<Component>>>> NeoEngine::components;
    std::vector<std::unique_ptr<System>> NeoEngine::systems;

    std::vector<std::unique_ptr<GameObject>> NeoEngine::gameObjectInitQueue;
    std::vector<GameObject *> NeoEngine::gameObjectKillQueue;
    std::vector<std::pair<std::type_index, std::unique_ptr<Component>>> NeoEngine::componentInitQueue;
    std::vector<std::pair<std::type_index, Component *>> NeoEngine::componentKillQueue;

    /* FPS */
    int NeoEngine::FPS = 0;
    int NeoEngine::nFrames = 0;
    int NeoEngine::totalFrames = 0;
    double NeoEngine::timeStep = 0.0;
    double NeoEngine::lastFPSTime = 0.0;
    double NeoEngine::lastFrameTime = 0.0;
    double NeoEngine::runTime = 0.0;

    /* ImGui */
    bool NeoEngine::imGuiEnabled = true;
    std::vector<std::function<void()>> NeoEngine::imGuiFuncs;

    void NeoEngine::init(const std::string &title, const std::string &app_res, const int width, const int height) {
        /* Init base engine */
        srand((unsigned int)(time(0)));
        APP_NAME = title;
        Loader::init(app_res, true);

        /* Init window*/
        if (Window::initGLFW(APP_NAME)) {
            std::cerr << "Failed initializing Window" << std::endl;
        }
        Window::setSize(glm::ivec2(width, height));

        /* Init FPS */
        lastFrameTime = runTime = glfwGetTime();
   }

    void NeoEngine::initSystems() {
        for (auto & system : systems) {
            system.get()->init();
        }
    }

    void NeoEngine::run() {

        while (!Window::shouldClose()) {
            /* Update delta time and FPS */
            updateFPS();

            /* Update display, mouse, keyboard */
            Window::update();

            /* Initialize new objects and components */
            processInitQueue();

            /* Update imgui functions */
            if (imGuiEnabled) {
                for (auto func : imGuiFuncs) {
                    func();
                }
            }

            /* Update each system */
            for (auto & system : systems) {
                system.get()->update((float)timeStep);
            }

            /* Render imgui */
            if (imGuiEnabled) {
                ImGui::Render();
            }

            /* Kill deleted objects and components */
            processKillQueue();
        }
    }

    GameObject & NeoEngine::createGameObject() {
        gameObjectInitQueue.emplace_back(std::make_unique<GameObject>());
        return *gameObjectInitQueue.back().get();
    }

    void NeoEngine::killGameObject(GameObject &go) {
        gameObjectKillQueue.push_back(&go);
    }

    void NeoEngine::processInitQueue() {
        initGameObjects();
        initComponents();
    }

    void NeoEngine::initGameObjects() {
        for (auto & object : gameObjectInitQueue) {
            gameObjects.emplace_back(std::move(object));
        }
        gameObjectInitQueue.clear();
    }

    void NeoEngine::initComponents() {
        for (int i = 0; i < int(componentInitQueue.size()); i++) {
            auto & type(componentInitQueue[i].first);
            auto & comp(componentInitQueue[i].second);
            /* Add Component to respective GameObjects */
            comp.get()->getGameObject().addComponent(*comp.get(), type);

            /* Add Component to active engine */
            auto it(components.find(type));
            if (it == components.end()) {
                components.emplace(type, std::make_unique<std::vector<std::unique_ptr<Component>>>());
                it = components.find(type);
            }
            it->second->emplace_back(std::move(comp));
            it->second->back()->init();
        }
        componentInitQueue.clear();
    }

    void NeoEngine::processKillQueue() {
        /* Remove Components from GameObjects */
        for (auto & comp : componentKillQueue) {
            comp.second->getGameObject().removeComponent(*(comp.second), comp.first);
        }

        killGameObjects();
        killComponents();
    }

    void NeoEngine::killGameObjects() {
        for (auto killIt(gameObjectKillQueue.begin()); killIt != gameObjectKillQueue.end(); ++killIt) {
            bool found = false;
            /* Look in active game objects in reverse order */
            for (int i = int(gameObjects.size()) - 1; i >= 0; --i) {
                GameObject * go(gameObjects[i].get());
                if (go == *killIt) {
                    /* Add game object's components to kill queue */
                    for (auto compTIt(go->compsByCompT.begin()); compTIt != go->compsByCompT.end(); ++compTIt) {
                        for (auto & comp : compTIt->second) {
                            comp->removeGameObject();
                            componentKillQueue.emplace_back(compTIt->first, comp);
                        }
                    }
                    gameObjects.erase(gameObjects.begin() + i);
                    found = true;
                    break;
                }
            }
            if (!found) {
                /* Look in game object initialization queue, in reverse order */
                for (int i = int(gameObjectInitQueue.size()) - 1; i >= 0; --i) {
                    if (gameObjectInitQueue[i].get() == *killIt) {
                        gameObjectInitQueue.erase(gameObjectInitQueue.begin() + i);
                        break;
                    }
                }
            }
        }
        gameObjectKillQueue.clear();
    }

    void NeoEngine::killComponents() {
        for (auto & killE : componentKillQueue) {
            std::type_index typeI(killE.first);
            Component * comp(killE.second);
            bool found(false);
            /* Look in active components in reverse order */
            if (components.count(typeI)) {
                auto & comps(*components.at(typeI));
                for (int i = int(comps.size()) - 1; i >= 0; --i) {
                    if (comps[i].get() == comp) {
                        auto it(comps.begin() + i);
                        comps.erase(it);
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                /* Look in component initialization queue in reverse order */
                for (int i = int(componentInitQueue.size()) - 1; i >= 0; --i) {
                    if (componentInitQueue[i].second.get() == comp) {
                        componentInitQueue.erase(componentInitQueue.begin() + i);
                        break;
                    }
                }
            }
        }
        componentKillQueue.clear();
    }

    void NeoEngine::shutDown() {
        Window::shutDown();
    }

    void NeoEngine::updateFPS() {
        runTime = glfwGetTime();
        totalFrames++;
        timeStep = runTime - lastFrameTime;
        lastFrameTime = runTime;
        nFrames++;
        if (runTime - lastFPSTime >= 1.0) {
            FPS = nFrames;
            nFrames = 0;
            lastFPSTime = runTime;
        }
    }
}