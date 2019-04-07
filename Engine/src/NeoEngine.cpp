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
    std::vector<std::unique_ptr<GameObject>> NeoEngine::mGameObjects;
    std::unordered_map<std::type_index, std::unique_ptr<std::vector<std::unique_ptr<Component>>>> NeoEngine::mComponents;
    std::vector<std::pair<std::type_index, std::unique_ptr<System>>> NeoEngine::mSystems;

    std::vector<std::unique_ptr<GameObject>> NeoEngine::mGameObjectInitQueue;
    std::vector<GameObject *> NeoEngine::mGameObjectKillQueue;
    std::vector<std::pair<std::type_index, std::unique_ptr<Component>>> NeoEngine::mComponentInitQueue;
    std::vector<std::pair<std::type_index, Component *>> NeoEngine::mComponentKillQueue;

    /* FPS */
    int Util::mFPS = 0;
    int Util::mFramesInCount = 0;
    int Util::mTotalFrames = 0;
    double Util::mTimeStep = 0.0;
    double Util::mLastFPSTime = 0.0;
    double Util::mLastFrameTime = 0.0;
    std::vector<int> Util::mFPSList;

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
        for (auto & system : mSystems) {
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
            _processInitQueue();
            Messenger::relayMessages();

            /* Update imgui functions */
            if (imGuiEnabled) {
                for (auto & it : imGuiFuncs) {
                    ImGui::Begin(it.first.c_str(), nullptr, ImVec2(0.f, 0.f), 0.5f, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_AlwaysAutoResize);
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
            MasterRenderer::render((float)Util::mTimeStep);

            /* Kill deleted objects and components */
            _processKillQueue();
            Messenger::relayMessages();
        }
    }

    GameObject & NeoEngine::createGameObject() {
        mGameObjectInitQueue.emplace_back(std::make_unique<GameObject>());
        return *mGameObjectInitQueue.back().get();
    }

    void NeoEngine::removeGameObject(GameObject &go) {
        mGameObjectKillQueue.push_back(&go);
    }

    void NeoEngine::_processInitQueue() {
        _initGameObjects();
        _initComponents();
    }

    void NeoEngine::_initGameObjects() {
        for (auto & object : mGameObjectInitQueue) {
            mGameObjects.emplace_back(std::move(object));
        }
        mGameObjectInitQueue.clear();
    }

    void NeoEngine::_initComponents() {

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

    void NeoEngine::_processKillQueue() {
        /* Remove Components from GameObjects */
        for (auto & comp : mComponentKillQueue) {
            comp.second->getGameObject().removeComponent(*(comp.second), comp.first);
        }

        _killGameObjects();
        _killComponents();
    }

    void NeoEngine::_killGameObjects() {
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

    void NeoEngine::_killComponents() {
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

    void NeoEngine::addDefaultImGuiFunc() {
        addImGuiFunc("Neo", [&]() {
            if (ImGui::CollapsingHeader("Performance")) {
                ImGui::PlotLines("FPS", (float *)Util::mFPSList.data(), Util::mFPSList.size(), 0, std::to_string(Util::mFPS).c_str(), FLT_MAX, FLT_MAX, ImVec2(0,0), sizeof(int));
                ImGui::Text("dt: %0.3fms", 1000.0 * Util::mTimeStep);
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
                if (mSystems.size() && ImGui::TreeNode("Systems")) {
                    for (unsigned i = 0; i < mSystems.size(); i++) {
                        auto & sys = mSystems[i].second;
                        ImGui::PushID(i);
                        ImGui::Checkbox(sys->mName.c_str(), &sys->mActive);

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
                    }
                    ImGui::TreePop();
                }
            }
            if (ImGui::CollapsingHeader("Renderer")) {
                if (MasterRenderer::mDefaultCamera && ImGui::TreeNode("Camera")) {
                    auto pos = MasterRenderer::mDefaultCamera->getGameObject().getSpatial()->getPosition();
                    auto look = MasterRenderer::mDefaultCamera->getLookDir();
                    ImGui::Text("Position: %0.2f, %0.2f, %0.2f", pos.x, pos.y, pos.z);
                    ImGui::Text("Look Dir: %0.2f, %0.2f, %0.2f", look.x, look.y, look.z);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Shaders")) {
                    int count = 0;
                    for (auto & r : MasterRenderer::mRenderables) {
                        count += r.second->size();
                    }
                    ImGui::Text("Renderables: %d", count); // TODO : list renderable count per shader 
                    if (MasterRenderer::mPreProcessShaders.size() && ImGui::TreeNode("Pre process")) {
                        for (unsigned i = 0; i < MasterRenderer::mPreProcessShaders.size(); i++) {
                            auto & shader = MasterRenderer::mPreProcessShaders[i];
                            ImGui::PushID(i);
                            ImGui::Checkbox(shader->mName.c_str(), &shader->mActive);

                            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                                ImGui::SetDragDropPayload("PRESHADER_SWAP", &i, sizeof(unsigned));
                                ImGui::Text("Swap %s", shader->mName.c_str());
                                ImGui::EndDragDropSource();
                            }
                            if (ImGui::BeginDragDropTarget()) {
                                if (const ImGuiPayload *payLoad = ImGui::AcceptDragDropPayload("PRESHADER_SWAP")) {
                                    IM_ASSERT(payLoad->DataSize == sizeof(unsigned));
                                    unsigned payload_n = *(const unsigned *)payLoad->Data;
                                    MasterRenderer::mPreProcessShaders[i].swap(MasterRenderer::mPreProcessShaders[payload_n]);
                                }
                                ImGui::EndDragDropTarget();
                            }
                            ImGui::PopID();
                        }
                        ImGui::TreePop();
                    }
                    if (MasterRenderer::mSceneShaders.size() && ImGui::TreeNode("Scene")) {
                        for (unsigned i = 0; i < MasterRenderer::mSceneShaders.size(); i++) {
                            auto & shader = MasterRenderer::mSceneShaders[i];
                            ImGui::PushID(i);
                            ImGui::Checkbox(shader->mName.c_str(), &shader->mActive);

                            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                                ImGui::SetDragDropPayload("SCENESHADER_SWAP", &i, sizeof(unsigned));
                                ImGui::Text("Swap %s", shader->mName.c_str());
                                ImGui::EndDragDropSource();
                            }
                            if (ImGui::BeginDragDropTarget()) {
                                if (const ImGuiPayload *payLoad = ImGui::AcceptDragDropPayload("SCENESHADER_SWAP")) {
                                    IM_ASSERT(payLoad->DataSize == sizeof(unsigned));
                                    unsigned payload_n = *(const unsigned *)payLoad->Data;
                                    MasterRenderer::mSceneShaders[i].swap(MasterRenderer::mSceneShaders[payload_n]);
                                }
                                ImGui::EndDragDropTarget();
                            }
                            ImGui::PopID();
                        }
                        ImGui::TreePop();
                    }
                    if (MasterRenderer::mPostShaders.size() && ImGui::TreeNode("Post process")) {
                        for (unsigned i = 0; i < MasterRenderer::mPostShaders.size(); i++) {
                            auto & shader = MasterRenderer::mPostShaders[i];
                            ImGui::PushID(i);
                            ImGui::Checkbox(shader->mName.c_str(), &shader->mActive);

                            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                                ImGui::SetDragDropPayload("POSTSHADER_SWAP", &i, sizeof(unsigned));
                                ImGui::Text("Swap %s", shader->mName.c_str());
                                ImGui::EndDragDropSource();
                            }
                            if (ImGui::BeginDragDropTarget()) {
                                if (const ImGuiPayload *payLoad = ImGui::AcceptDragDropPayload("POSTSHADER_SWAP")) {
                                    IM_ASSERT(payLoad->DataSize == sizeof(unsigned));
                                    unsigned payload_n = *(const unsigned *)payLoad->Data;
                                    MasterRenderer::mPostShaders[i].swap(MasterRenderer::mPostShaders[payload_n]);
                                }
                                ImGui::EndDragDropTarget();
                            }
                            ImGui::PopID();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
            if (ImGui::CollapsingHeader("Library")) {
                if (ImGui::TreeNode("FBOs")) {
                    for (auto & fbo : Loader::mFramebuffers) {
                        if (ImGui::TreeNode((fbo.first + " (" + std::to_string(fbo.second->mFBOID) + ")").c_str())) {
                            for (auto & t : fbo.second->mTextures) {
                                if (ImGui::TreeNode((std::to_string(t->mTextureID) + " [" + std::to_string(t->mWidth) + ", " + std::to_string(t->mHeight) + "]").c_str())) {
                                    float scale = 150.f / (t->mWidth > t->mHeight ? t->mWidth : t->mHeight);
                                    ImGui::Image((ImTextureID)t->mTextureID, ImVec2(scale * t->mWidth, scale * t->mHeight), ImVec2(0, 1), ImVec2(1, 0));
                                    ImGui::TreePop();
                                }
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                if (Loader::mMeshes.size() && ImGui::TreeNode("Meshes")) {
                    for (auto & m : Loader::mMeshes) {
                        ImGui::Text("%s (%d)", m.first.c_str(), m.second->mVertexBufferSize);
                    }
                    ImGui::TreePop();
                }
                if (Loader::mTextures.size() && ImGui::TreeNode("Textures")) {
                    for (auto & t : Loader::mTextures) {
                        if (ImGui::TreeNode((t.first + " (" + std::to_string(t.second->mTextureID) + ")" + " [" + std::to_string(t.second->mWidth) + ", " + std::to_string(t.second->mHeight) + "]").c_str())) {
                            float scale = 150.f / (t.second->mWidth > t.second->mHeight ? t.second->mWidth : t.second->mHeight);
                            ImGui::Image((ImTextureID)t.second->mTextureID, ImVec2(scale * t.second->mWidth, scale * t.second->mHeight), ImVec2(0, 1), ImVec2(1, 0));
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                if (Loader::mMaterials.size() && ImGui::TreeNode("Materials")) {
                    for (auto & mat : Loader::mMaterials) {
                        if (ImGui::TreeNode(mat.first.c_str())) {
                            ImGui::SliderFloat("Ambient", &mat.second->mAmbient, 0.f, 1.f);
                            ImGui::SliderFloat3("Diffuse", glm::value_ptr(mat.second->mDiffuse), 0.f, 1.f);
                            ImGui::SliderFloat3("Specular", glm::value_ptr(mat.second->mSpecular), 0.f, 1.f);
                            ImGui::SliderFloat("Shine", &mat.second->mShine, 0.f, 100.f);
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