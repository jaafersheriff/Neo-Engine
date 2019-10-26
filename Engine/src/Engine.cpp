// allows program to be run on dedicated graphics processor for laptops with
// both integrated and dedicated graphics using Nvidia Optimus
#ifdef _WIN32
extern "C" {
    _declspec(dllexport) unsigned int NvOptimusEnablement = 0x00000001;
}
#endif

#include "Engine.hpp"
#include "Renderer/Renderer.hpp"
#include "Shader/WireframeShader.hpp"
#include "Shader/OutlineShader.hpp"

#include "GameObject/GameObject.hpp"
#include "Messaging/Messenger.hpp"

#include "Loader/Loader.hpp"

#include <time.h>
#include <iostream>

namespace neo {

    /* Base Engine */
    EngineConfig Engine::mConfig;

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
    bool Engine::mImGuiEnabled = true;
    std::unordered_map<std::string, std::function<void()>> Engine::mImGuiFuncs;

    void Engine::init(EngineConfig config) {
        /* Init base engine */
        srand((unsigned int)(time(0)));
        mConfig = config;

        /* Init window*/
        if (Window::initGLFW(mConfig.APP_NAME)) {
            std::cerr << "Failed initializing Window" << std::endl;
        }
        Window::setSize(glm::ivec2(mConfig.width, mConfig.height));

        /* Init loader after initializing GL*/
        Loader::init(mConfig.APP_RES, true);

        /* Generate basic meshes */
        Library::getMesh("cube");
        Library::getMesh("quad");
        Library::getMesh("sphere");

        /* Init Util */
        Util::init();
    }

    void Engine::run() {
        /* Initialize new objects and components */
        _processInitQueue();
        Messenger::relayMessages();

        /* Init systems */
        _initSystems();

        if (mConfig.attachEditor) {
            addSystem<MouseRaySystem>();
            addSystem<EditorSystem>();
            Renderer::addSceneShader<WireframeShader>();
            Renderer::addSceneShader<OutlineShader>();
            ImGui::GetStyle().ScaleAllSizes(2.f);
        }

        while (!Window::shouldClose()) {
            /* Update Util */
            Util::update();

            /* Update display, mouse, keyboard */
            Window::update();

            /* Destroy and create objects and components */
            _processKillQueue();
            _processInitQueue();
            Messenger::relayMessages();

            /* Update each system */
            for (auto & system : mSystems) {
                if (system.second->mActive) {
                    system.second->update((float)Util::mTimeStep);
                    Messenger::relayMessages();
                }
            }
 
            /* Update imgui functions */
            if (mImGuiEnabled) {
                ImGui::GetIO().FontGlobalScale = 2.0f;
                _runImGui();
                Messenger::relayMessages();
            }

            /* Render */
            // TODO - only run this at 60FPS in its own thread
            // TODO - should this go after processkillqueue?
            Renderer::render((float)Util::mTimeStep);
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

    void Engine::_initSystems() {
        for (auto & system : mSystems) {
            system.second->init();
        }
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

    void Engine::_runImGui() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Performance")) {
                // Translate FPS to floats
                std::vector<float> FPSfloats(Util::mFPSList.begin(), Util::mFPSList.end());
                ImGui::PlotLines("FPS", FPSfloats.data(), FPSfloats.size(), 0, std::to_string(Util::mFPS).c_str());
                ImGui::Text("dt: %0.3fms", 1000.0 * Util::mTimeStep);
                if (ImGui::Button("VSync")) {
                    Window::toggleVSync();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("ECS")) {
                ImGui::Text("GameObjects:  %d", Engine::getGameObjects().size());
                int count = 0;
                for (auto go : Engine::getGameObjects()) {
                    count += go->getAllComponents().size();
                }
                ImGui::Text("Components:  %d", count);
                if (mSystems.size() && ImGui::TreeNodeEx("Systems", ImGuiTreeNodeFlags_DefaultOpen)) {
                    for (unsigned i = 0; i < mSystems.size(); i++) {
                        auto & sys = mSystems[i].second;
                        ImGui::PushID(i);
                        bool treeActive = ImGui::TreeNodeEx(sys->mName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
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
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Renderer")) {
                if (Renderer::mDefaultCamera && ImGui::TreeNode("Scene Camera")) {
                    auto spatial = Renderer::mDefaultCamera->getGameObject().getComponentByType<SpatialComponent>();
                    assert(spatial);
                    auto pos = spatial->getPosition();
                    auto look = spatial->getLookDir();
                    ImGui::Text("Position: %0.2f, %0.2f, %0.2f", pos.x, pos.y, pos.z);
                    ImGui::Text("Look Dir: %0.2f, %0.2f, %0.2f", look.x, look.y, look.z);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNodeEx("Shaders", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto shadersFunc = [&](std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>>& shaders, const std::string swapName) {
                        for (unsigned i = 0; i < shaders.size(); i++) {
                            auto& shader = shaders[i];
                            ImGui::PushID(i);
                            bool treeActive = ImGui::TreeNodeEx(shader.second->mName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
                            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                                ImGui::SetDragDropPayload(swapName.c_str(), &i, sizeof(unsigned));
                                ImGui::Text("Swap %s", shader.second->mName.c_str());
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
                                ImGui::Checkbox("Active", &shader.second->mActive);
                                ImGui::SameLine();
                                if (ImGui::Button("Reload")) {
                                    shader.second->reload();
                                }
                                shader.second->imguiEditor();
                                ImGui::TreePop();
                            }
                            ImGui::PopID();
                        }
                    };

                    if (Renderer::mPreProcessShaders.size() && ImGui::TreeNodeEx("Pre process", ImGuiTreeNodeFlags_DefaultOpen)) {
                        shadersFunc(Renderer::mPreProcessShaders, "PRESHADER_SWAP");
                        ImGui::TreePop();
                    }
                    if (Renderer::mSceneShaders.size() && ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
                        shadersFunc(Renderer::mSceneShaders, "SCENESHADER_SWAP");
                        ImGui::TreePop();
                    }
                    if (Renderer::mPostShaders.size() && ImGui::TreeNodeEx("Post process", ImGuiTreeNodeFlags_DefaultOpen)) {
                        shadersFunc(Renderer::mPostShaders, "POSTSHADER_SWAP");
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                ImGui::EndMenu();
            }
            auto textureFunc = [&](const Texture& texture) {
                float scale = 150.f / (texture.mWidth > texture.mHeight ? texture.mWidth : texture.mHeight);
                ImGui::Image((ImTextureID)texture.mTextureID, ImVec2(scale * texture.mWidth, scale * texture.mHeight), ImVec2(0, 1), ImVec2(1, 0));
            };
            if (ImGui::BeginMenu("Library")) {
                if (ImGui::TreeNodeEx("FBOs", ImGuiTreeNodeFlags_DefaultOpen)) {
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
                if (ImGui::TreeNodeEx("Meshes", ImGuiTreeNodeFlags_DefaultOpen)) {
                    for (auto & m : Library::mMeshes) {
                        ImGui::Text("%s (%d)", m.first.c_str(), m.second->mVertexBufferSize);
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNodeEx("Textures", ImGuiTreeNodeFlags_DefaultOpen)) {
                    for (auto& t : Library::mTextures) {
                        if (ImGui::TreeNode((t.first + " (" + std::to_string(t.second->mTextureID) + ")" + " [" + std::to_string(t.second->mWidth) + ", " + std::to_string(t.second->mHeight) + "]").c_str())) {
                            textureFunc(*t.second);
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                ImGui::EndMenu();
            }
            if (mConfig.attachEditor && ImGui::BeginMenu("Editor")) {
                if (auto selected = Engine::getSingleComponent<SelectedComponent>()) {
                    auto allComponents = selected->getGameObject().getComponentsMap();
                    static std::optional<std::type_index> index;
                    if (ImGui::BeginCombo("", index ? index->name() + 6 : "Edit components")) {
                        index = std::nullopt;
                        for (auto comp : allComponents) {
                            if (comp.second.size()) {
                                if (ImGui::Selectable(comp.first.name() + 6)) {
                                    index = comp.first;
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                    if (index) {
                        auto components = allComponents[index.value()];
                        if (components.size()) {
                            static int offset = 0;
                            if (components.size() > 1) {
								ImGui::Indent();
                                ImGui::SliderInt("Index", &offset, 0, components.size() - 1);
								ImGui::Unindent();
                            }
							ImGui::Indent();
                            components[offset]->imGuiEditor();
							ImGui::Unindent();

							ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.81f, 0.20f, 0.20f, 0.40f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.81f, 0.20f, 0.20f, 1.00f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.81f, 0.15f, 0.05f, 1.00f));
                            if (ImGui::Button("Remove", ImVec2(ImGui::GetWindowWidth() * 0.9f, 0))) {
								// TODO...
								// Engine::removeComponent(static_cast<decltype(*index)>(components[offset]));
                                index = std::nullopt;
                            }
							ImGui::PopStyleColor(3);
                        }
                    }
					ImGui::Separator();
                    if (ImGui::BeginCombo("", "Add components")) {
                        // TODO..
                        ImGui::EndCombo();
                    }
                }
				ImGui::Separator();
                if (ImGui::Button("Create new GameObject")) {
                    Engine::removeComponent<SelectedComponent>(*Engine::getSingleComponent<SelectedComponent>());
                    auto& go = Engine::createGameObject();
                    Engine::addComponent<BoundingBoxComponent>(&go, std::vector<float>{ -1.f, -1.f, -1.f, 1.f, 1.f, 1.f });
                    Engine::addComponent<SpatialComponent>(&go);
                    Engine::addComponent<SelectableComponent>(&go);
                    Engine::addComponent<SelectedComponent>(&go);
                    Engine::addComponent<MeshComponent>(&go, Library::getMesh("sphere"));
                    Engine::addComponent<renderable::WireframeRenderable>(&go);
                }
 
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(mConfig.APP_NAME.c_str())) {
                for (auto & it : mImGuiFuncs) {
                    if (ImGui::TreeNodeEx(it.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                        it.second();
                        ImGui::TreePop();
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void Engine::shutDown() {
        Window::shutDown();
    }

}