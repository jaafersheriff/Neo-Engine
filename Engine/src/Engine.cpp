// allows program to be run on dedicated graphics processor for laptops with
// both integrated and dedicated graphics using Nvidia Optimus
#ifdef _WIN32
extern "C" {
    _declspec(dllexport) unsigned int NvOptimusEnablement = 0x00000001;
}
#endif

#include "Engine.hpp"
#include "Renderer/Renderer.hpp"

#include "GameObject/GameObject.hpp"
#include "Messaging/Messenger.hpp"

#include "Loader/Loader.hpp"

#include <time.h>
#include <iostream>

namespace neo {

    /* Base Engine */
    std::string Engine::APP_NAME = "Neo Engine";

    /* ECS */
    std::vector<std::unique_ptr<GameObject>> Engine::mGameObjects;
    std::unordered_map<std::type_index, std::unique_ptr<std::vector<std::unique_ptr<Component>>>> Engine::mComponents;
    std::vector<std::pair<std::type_index, std::unique_ptr<System>>> Engine::mSystems;

    std::vector<std::unique_ptr<GameObject>> Engine::mGameObjectInitQueue;
    std::vector<GameObject *> Engine::mGameObjectKillQueue;
    std::vector<std::pair<std::type_index, std::unique_ptr<Component>>> Engine::mComponentInitQueue;
    std::vector<std::pair<std::type_index, Component *>> Engine::mComponentKillQueue;

    /* FPS */
    int Util::mFPS = 0;
    int Util::mFramesInCount = 0;
    int Util::mTotalFrames = 0;
    double Util::mTimeStep = 0.0;
    double Util::mLastFPSTime = 0.0;
    double Util::mLastFrameTime = 0.0;
    std::vector<int> Util::mFPSList;

    /* ImGui */
    bool Engine::imGuiEnabled = true;
    std::unordered_map<std::string, std::function<void()>> Engine::imGuiFuncs;

    void Engine::init(const std::string &title, const std::string &app_res, const int width, const int height) {
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

    void Engine::initSystems() {
        for (auto & system : mSystems) {
            system.second->init();
        }
    }

    void Engine::run() {

        while (!Window::shouldClose()) {
            /* Update Util */
            Util::update();

            /* Update display, mouse, keyboard */
            Window::update();

            /* Initialize new objects and components */
            _processInitQueue();
            Messenger::relayMessages();

            /* Update imgui functions */
            if (imGuiEnabled) {
                for (auto & it : imGuiFuncs) {
                    ImGui::Begin(it.first.c_str(), nullptr, ImVec2(0.f, 0.f), 0.5f, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_AlwaysAutoResize);
                    ImGui::SetWindowFontScale(1.3f);
                    it.second();
                    ImGui::End();
                }
                Messenger::relayMessages();
            }

            /* Update each system */
            for (auto & system : mSystems) {
                if (system.second->mActive) {
                    system.second->update((float)Util::mTimeStep);
                    Messenger::relayMessages();
                }
            }

            /* Render */
            // TODO - only run this at 60FPS in its own thread
            // TODO - should this go after processkillqueue?
            Renderer::render((float)Util::mTimeStep);

            /* Kill deleted objects and components */
            _processKillQueue();
            Messenger::relayMessages();
        }
    }

    GameObject & Engine::createGameObject() {
        mGameObjectInitQueue.emplace_back(std::make_unique<GameObject>());
        return *mGameObjectInitQueue.back().get();
    }

    void Engine::removeGameObject(GameObject &go) {
        mGameObjectKillQueue.push_back(&go);
    }

    void Engine::_processInitQueue() {
        _initGameObjects();
        _initComponents();
    }

    void Engine::_initGameObjects() {
        for (auto & object : mGameObjectInitQueue) {
            mGameObjects.emplace_back(std::move(object));
        }
        mGameObjectInitQueue.clear();
    }

    void Engine::_initComponents() {

        for (int i = 0; i < int(mComponentInitQueue.size()); i++) {
            auto & type(mComponentInitQueue[i].first);
            auto & comp(mComponentInitQueue[i].second);
            /* Add Component to respective GameObjects */
            comp.get()->getGameObject().addComponent(*comp.get(), type);

            /* Add Component to active engine */
            auto it(mComponents.find(type));
            if (it == mComponents.end()) {
                mComponents.emplace(type, std::make_unique<std::vector<std::unique_ptr<Component>>>());
                it = mComponents.find(type);
            }
            it->second->emplace_back(std::move(comp));
            it->second->back()->init();
        }
        mComponentInitQueue.clear();
    }

    void Engine::_processKillQueue() {
        /* Remove Components from GameObjects */
        for (auto & comp : mComponentKillQueue) {
            comp.second->getGameObject().removeComponent(*(comp.second), comp.first);
        }

        _killGameObjects();
        _killComponents();
    }

    void Engine::_killGameObjects() {
        for (auto killIt(mGameObjectKillQueue.begin()); killIt != mGameObjectKillQueue.end(); ++killIt) {
            bool found = false;
            /* Look in active game objects in reverse order */
            for (int i = int(mGameObjects.size()) - 1; i >= 0; --i) {
                GameObject * go(mGameObjects[i].get());
                if (go == *killIt) {
                    /* Add game object's components to kill queue */
                    for (auto compTIt(go->mComponentsByType.begin()); compTIt != go->mComponentsByType.end(); ++compTIt) {
                        for (auto & comp : compTIt->second) {
                            comp->removeGameObject();
                            mComponentKillQueue.emplace_back(compTIt->first, comp);
                        }
                    }
                    mGameObjects.erase(mGameObjects.begin() + i);
                    found = true;
                    break;
                }
            }
            if (!found) {
                /* Look in game object initialization queue, in reverse order */
                for (int i = int(mGameObjectInitQueue.size()) - 1; i >= 0; --i) {
                    if (mGameObjectInitQueue[i].get() == *killIt) {
                        mGameObjectInitQueue.erase(mGameObjectInitQueue.begin() + i);
                        break;
                    }
                }
            }
        }
        mGameObjectKillQueue.clear();
    }

    void Engine::_killComponents() {
        for (auto & killE : mComponentKillQueue) {
            std::type_index typeI(killE.first);
            Component * comp(killE.second);
            comp->kill();
            bool found(false);
            /* Look in active components in reverse order */
            if (mComponents.count(typeI)) {
                auto & comps(*mComponents.at(typeI));
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
                for (int i = int(mComponentInitQueue.size()) - 1; i >= 0; --i) {
                    if (mComponentInitQueue[i].second.get() == comp) {
                        mComponentInitQueue.erase(mComponentInitQueue.begin() + i);
                        break;
                    }
                }
            }
        }
        mComponentKillQueue.clear();
    }

    void Engine::addDefaultImGuiFunc() {
        addImGuiFunc("Neo", [&]() {
            if (ImGui::CollapsingHeader("Performance")) {
                // Translate FPS to floats
                std::vector<float> FPSfloats(Util::mFPSList.begin(), Util::mFPSList.end());
                ImGui::PlotLines("FPS", FPSfloats.data(), FPSfloats.size(), 0, std::to_string(Util::mFPS).c_str());
                ImGui::Text("dt: %0.3fms", 1000.0 * Util::mTimeStep);
                if (ImGui::Button("VSync")) {
                    Window::toggleVSync();
                }
            }
            if (ImGui::CollapsingHeader("ECS")) {
                ImGui::Text("GameObjects:  %d", Engine::getGameObjects().size());
                int count = 0;
                for (auto go : Engine::getGameObjects()) {
                    count += go->getAllComponents().size();
                }
                ImGui::Text("Components:  %d", count);
                if (mSystems.size() && ImGui::TreeNode("Systems")) {
                    for (unsigned i = 0; i < mSystems.size(); i++) {
                        auto & sys = mSystems[i].second;
                        ImGui::PushID(i);
                        bool treeActive = ImGui::TreeNode(sys->mName.c_str());
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                            ImGui::SetDragDropPayload("SYSTEM_SWAP", &i, sizeof(unsigned));
                            ImGui::Text("Swap %s", sys->mName.c_str());
                            ImGui::EndDragDropSource();
                        }
                        if (ImGui::BeginDragDropTarget()) {
                            if (const ImGuiPayload *payLoad = ImGui::AcceptDragDropPayload("SYSTEM_SWAP")) {
                                IM_ASSERT(payLoad->DataSize == sizeof(unsigned));
                                unsigned payload_n = *(const unsigned *)payLoad->Data;
                                mSystems[i].swap(mSystems[payload_n]);
                            }
                            ImGui::EndDragDropTarget();
                        }
                        ImGui::PopID();
                        if (treeActive) {
                            ImGui::Checkbox("Active", &sys->mActive);
                            sys->imguiEditor();
                            ImGui::TreePop();
                        }
                    }

                    ImGui::TreePop();
                }
            }
            if (ImGui::CollapsingHeader("Renderer")) {
                if (Renderer::mDefaultCamera && ImGui::TreeNode("Camera")) {
                    auto pos = Renderer::mDefaultCamera->getGameObject().getSpatial()->getPosition();
                    auto look = Renderer::mDefaultCamera->getGameObject().getSpatial()->getLookDir();
                    ImGui::Text("Position: %0.2f, %0.2f, %0.2f", pos.x, pos.y, pos.z);
                    ImGui::Text("Look Dir: %0.2f, %0.2f, %0.2f", look.x, look.y, look.z);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Shaders")) {
                    auto shadersFunc = [&](std::vector<std::unique_ptr<Shader>>& shaders, const std::string swapName) {
                        for (unsigned i = 0; i < shaders.size(); i++) {
                            auto& shader = shaders[i];
                            ImGui::PushID(i);
                            bool treeActive = ImGui::TreeNode(shader->mName.c_str());
                            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                                ImGui::SetDragDropPayload(swapName.c_str(), &i, sizeof(unsigned));
                                ImGui::Text("Swap %s", shader->mName.c_str());
                                ImGui::EndDragDropSource();
                            }
                            if (ImGui::BeginDragDropTarget()) {
                                if (const ImGuiPayload *payLoad = ImGui::AcceptDragDropPayload(swapName.c_str())) {
                                    IM_ASSERT(payLoad->DataSize == sizeof(unsigned));
                                    unsigned payload_n = *(const unsigned *)payLoad->Data;
                                    shaders[i].swap(shaders[payload_n]);
                                }
                                ImGui::EndDragDropTarget();
                            }

                            if (treeActive) {
                                ImGui::Checkbox("Active", &shader->mActive);
                                ImGui::SameLine();
                                if (ImGui::Button("Reload")) {
                                    shader->reload();
                                }
                                shader->imguiEditor();
                                ImGui::TreePop();
                            }
                            ImGui::PopID();
                        }
                    };

                    if (Renderer::mPreProcessShaders.size() && ImGui::TreeNode("Pre process")) {
                        shadersFunc(Renderer::mPreProcessShaders, "PRESHADER_SWAP");
                        ImGui::TreePop();
                    }
                    if (Renderer::mSceneShaders.size() && ImGui::TreeNode("Scene")) {
                        shadersFunc(Renderer::mSceneShaders, "SCENESHADER_SWAP");
                        ImGui::TreePop();
                    }
                    if (Renderer::mPostShaders.size() && ImGui::TreeNode("Post process")) {
                        shadersFunc(Renderer::mPostShaders, "POSTSHADER_SWAP");
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
            auto textureFunc = [&](const Texture& texture) {
                float scale = 150.f / (texture.mWidth > texture.mHeight ? texture.mWidth : texture.mHeight);
                ImGui::Image((ImTextureID)texture.mTextureID, ImVec2(scale * texture.mWidth, scale * texture.mHeight), ImVec2(0, 1), ImVec2(1, 0));
            };
            if (ImGui::CollapsingHeader("Library")) {
                if (Library::mFramebuffers.size() && ImGui::TreeNode("FBOs")) {
                    for (auto & fbo : Library::mFramebuffers) {
                        if (ImGui::TreeNode((fbo.first + " (" + std::to_string(fbo.second->mFBOID) + ")").c_str())) {
                            for (auto& t : fbo.second->mTextures) {
                                if (ImGui::TreeNode((std::to_string(t->mTextureID) + " [" + std::to_string(t->mWidth) + ", " + std::to_string(t->mHeight) + "]").c_str())) {
                                    textureFunc(*t);
                                    ImGui::TreePop();
                                }
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                if (Library::mMeshes.size() && ImGui::TreeNode("Meshes")) {
                    for (auto & m : Library::mMeshes) {
                        ImGui::Text("%s (%d)", m.first.c_str(), m.second->mVertexBufferSize);
                    }
                    ImGui::TreePop();
                }
                if (Library::mTextures.size() && ImGui::TreeNode("Textures")) {
                    for (auto& t : Library::mTextures) {
                        if (ImGui::TreeNode((t.first + " (" + std::to_string(t.second->mTextureID) + ")" + " [" + std::to_string(t.second->mWidth) + ", " + std::to_string(t.second->mHeight) + "]").c_str())) {
                            textureFunc(*t.second);
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
            }
        });
    }

    void Engine::shutDown() {
        Window::shutDown();
    }

}