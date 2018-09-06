// allows program to be run on dedicated graphics processor for laptops with
// both integrated and dedicated graphics using Nvidia Optimus
#ifdef _WIN32
extern "C" {
    _declspec(dllexport) unsigned int NvOptimusEnablement = 0x00000001;
}
#endif

#include "NeoEngine.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

#include "GameObject/GameObject.hpp"
#include "Messaging/Messenger.hpp"

#include <time.h>
#include <iostream>

namespace neo {

    /* Base Engine */
    std::string NeoEngine::APP_NAME = "Neo Engine";

    /* ECS */
    std::vector<std::unique_ptr<GameObject>> NeoEngine::gameObjects;
    std::unordered_map<std::type_index, std::unique_ptr<std::vector<std::unique_ptr<Component>>>> NeoEngine::components;
    std::unordered_map<std::type_index, std::unique_ptr<System>> NeoEngine::systems;

    std::vector<std::unique_ptr<GameObject>> NeoEngine::gameObjectInitQueue;
    std::vector<GameObject *> NeoEngine::gameObjectKillQueue;
    std::vector<std::pair<std::type_index, std::unique_ptr<Component>>> NeoEngine::componentInitQueue;
    std::vector<std::pair<std::type_index, Component *>> NeoEngine::componentKillQueue;

    /* FPS */
    int Util::FPS = 0;
    int Util::nFrames = 0;
    int Util::totalFrames = 0;
    double Util::timeStep = 0.0;
    double Util::lastFPSTime = 0.0;
    double Util::lastFrameTime = 0.0;
    std::vector<int> Util::FPSList;

    /* ImGui */
    bool NeoEngine::imGuiEnabled = true;
    std::unordered_map<std::string, std::function<void()>> NeoEngine::imGuiFuncs;

    void NeoEngine::init(const std::string &title, const std::string &app_res, const int width, const int height) {
        /* Init base engine */
        srand((unsigned int)(time(0)));
        APP_NAME = title;

        /* Init window*/
        if (Window::initGLFW(APP_NAME)) {
            std::cerr << "Failed initializing Window" << std::endl;
        }
        Window::setSize(glm::ivec2(width, height));

        /* Init loader after initializing GL*/
        Loader::init(app_res, true);

        /* Init Util */
        Util::init();
   }

    void NeoEngine::initSystems() {
        for (auto & system : systems) {
            system.second->init();
        }
    }

    void NeoEngine::run() {

        while (!Window::shouldClose()) {
            /* Update Util */
            Util::update();

            /* Update display, mouse, keyboard */
            Window::update();

            /* Initialize new objects and components */
            processInitQueue();
            Messenger::relayMessages();

            /* Update imgui functions */
            if (imGuiEnabled) {
                for (auto it = imGuiFuncs.begin(); it != imGuiFuncs.end(); ++it) {
                    ImGui::Begin(it->first.c_str(), nullptr, ImVec2(0.f, 0.f), 0.5f, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_AlwaysAutoResize);
                    it->second();
                    ImGui::End();
                }
                Messenger::relayMessages();
            }

            /* Update each system */
            for (auto & system : systems) {
                if (system.second->active) {
                    system.second->update((float)Util::timeStep);
                    Messenger::relayMessages();
                }
            }

            /* Render */
            // TODO - only run this at 60FPS in its own thread
            // TODO - should this go after processkillqueue?
            MasterRenderer::render((float)Util::timeStep);

            /* Kill deleted objects and components */
            processKillQueue();
            Messenger::relayMessages();
        }
    }

    GameObject & NeoEngine::createGameObject() {
        gameObjectInitQueue.emplace_back(std::make_unique<GameObject>());
        return *gameObjectInitQueue.back().get();
    }

    void NeoEngine::removeGameObject(GameObject &go) {
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
            comp->kill();
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

    void NeoEngine::addDefaultImGuiFunc() {
        addImGuiFunc("Neo", [&]() {
            if (ImGui::CollapsingHeader("Performance")) {
                ImGui::PlotLines("FPS", (float *)Util::FPSList.data(), Util::FPSList.size(), 0, std::to_string(Util::FPS).c_str(), FLT_MAX, FLT_MAX, ImVec2(0,0), sizeof(int));
                ImGui::Text("dt: %0.3fms", 1000.0 * Util::timeStep);
                if (ImGui::Button("VSync")) {
                    Window::toggleVSync();
                }
            }
            if (ImGui::CollapsingHeader("ECS")) {
                ImGui::Text("GameObjects:  %d", NeoEngine::getGameObjects().size());
                int count = 0;
                for (auto go : NeoEngine::getGameObjects()) {
                    count += go->getAllComponents().size();
                }
                ImGui::Text("Components:  %d", count);
                if (ImGui::TreeNode("Systems")) {
                    for (auto sys : NeoEngine::getSystems()) {
                        ImGui::Checkbox(sys.second->name.c_str(), &sys.second->active);
                    }
                    ImGui::TreePop();
                }
            }
            if (ImGui::CollapsingHeader("Renderer")) {
                if (MasterRenderer::defaultCamera && ImGui::TreeNode("Camera")) {
                    auto pos = MasterRenderer::defaultCamera->getGameObject().getSpatial()->getPosition();
                    auto look = MasterRenderer::defaultCamera->getLookDir();
                    ImGui::Text("Position: %0.2f, %0.2f, %0.2f", pos.x, pos.y, pos.z);
                    ImGui::Text("Look Dir: %0.2f, %0.2f, %0.2f", look.x, look.y, look.z);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Shaders")) {
                    // TODO : list renderable count per shader 
                    int count = 0;
                    for (auto & r : MasterRenderer::renderables) {
                        count += r.second->size();
                    }
                    ImGui::Text("Renderables: %d", count);
                    if (MasterRenderer::preShaders.size() && ImGui::TreeNode("Pre process")) {
                        for (auto & shader : MasterRenderer::preShaders) {
                            ImGui::Checkbox(shader->name.c_str(), &shader->active);
                        }
                        ImGui::TreePop();
                    }
                    if (MasterRenderer::sceneShaders.size() && ImGui::TreeNode("Scene")) {
                        for (auto & shader : MasterRenderer::sceneShaders) {
                            ImGui::Checkbox(shader->name.c_str(), &shader->active);
                        }
                        ImGui::TreePop();
                    }
                    if (MasterRenderer::postShaders.size() && ImGui::TreeNode("Post process")) {
                        for (auto & shader : MasterRenderer::postShaders) {
                            ImGui::Checkbox(shader->name.c_str(), &shader->active);
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
            if (ImGui::CollapsingHeader("Library")) {
                // TODO : move fbos to library
                if (ImGui::TreeNode("FBOs")) {
                    for (auto & fbo : MasterRenderer::framebuffers) {
                        if (ImGui::TreeNode((fbo.first + " (" + std::to_string(fbo.second->fboId) + ")").c_str())) {
                            for (auto & t : fbo.second->textures) {
                                if (ImGui::TreeNode(std::to_string(t->textureId).c_str())) {
                                    float scale = 150.f / (t->width > t->height ? t->width : t->height);
                                    ImGui::Image((ImTextureID)t->textureId, ImVec2(scale * t->width, scale * t->height), ImVec2(0, 1), ImVec2(1, 0));
                                    ImGui::TreePop();
                                }
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                if (Loader::meshes.size() && ImGui::TreeNode("Meshes")) {
                    for (auto & m : Loader::meshes) {
                        ImGui::Text("%s (%d)", m.first.c_str(), m.second->vertBufSize);
                    }
                    ImGui::TreePop();
                }
                if (Loader::textures.size() && ImGui::TreeNode("Textures")) {
                    for (auto & t : Loader::textures) {
                        if (ImGui::TreeNode((t.first + " (" + std::to_string(t.second->textureId) + ")").c_str())) {
                            float scale = 150.f / (t.second->width > t.second->height ? t.second->width : t.second->height);
                            ImGui::Image((ImTextureID)t.second->textureId, ImVec2(scale * t.second->width, scale * t.second->height), ImVec2(0, 1), ImVec2(1, 0));
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
            }
        });
    }

    void NeoEngine::shutDown() {
        Window::shutDown();
    }

}