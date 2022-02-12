// allows program to be run on dedicated graphics processor for laptops with
// both integrated and dedicated graphics using Nvidia Optimus
#ifdef _WIN32
extern "C" {
    _declspec(dllexport) unsigned int NvOptimusEnablement = 0x00000001;
}
#endif

#include "Engine.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/Shader/SelectableShader.hpp"
#include "Renderer/Shader/OutlineShader.hpp"
#include "Renderer/Shader/WireframeShader.hpp"
#include "Renderer/Shader/LineShader.hpp"

#include "Hardware/WindowSurface.hpp"

#include "Messaging/Messenger.hpp"

#include "Loader/Loader.hpp"
#include "Loader/MeshGenerator.hpp"

#include "Util/FrameCounter.hpp"

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

    /* ImGui */
    bool Engine::mImGuiEnabled = true;
    std::unordered_map<std::string, std::function<void()>> Engine::mImGuiFuncs;

    /* Hardware */
    WindowSurface Engine::mWindow;
    Keyboard Engine::mKeyboard;
    Mouse Engine::mMouse;

    void Engine::init(EngineConfig config) {

        /* Init base engine */
        srand((unsigned int)(time(0)));
        mConfig = config;

        /* Init window*/
        if (mWindow.init(mConfig.APP_NAME)) {
            std::cerr << "Failed initializing Window" << std::endl;
        }
        mWindow.setSize(glm::ivec2(mConfig.width, mConfig.height));
        ImGui::GetStyle().ScaleAllSizes(2.f);

        mMouse.init();
        mKeyboard.init();

        /* Init loader after initializing GL*/
        Loader::init(mConfig.APP_RES, true);

        /* Generate basic meshes */
        Mesh* mesh = new Mesh;
        MeshGenerator::generateCube(mesh);
        Library::loadMesh(std::string("cube"), mesh);
        mesh = new Mesh;
        MeshGenerator::generateQuad(mesh);
        Library::loadMesh(std::string("quad"), mesh);
        mesh = new Mesh;
        MeshGenerator::generateSphere(mesh, 2);
        Library::loadMesh(std::string("sphere"), mesh);

        /* Generate basic textures*/
        uint8_t data[] = { 0x00, 0x00, 0x00, 0xFF };
        auto tex = Library::createEmptyTexture<Texture2D>("black", {});
        tex->update(glm::uvec2(1), data);
        data[0] = data[1] = data[2] = 0xFF;
        tex = Library::createEmptyTexture<Texture2D>("white", {});
        tex->update(glm::uvec2(1), data);

#if MICROPROFILE_ENABLED
        MicroProfileOnThreadCreate("MAIN THREAD");
        MicroProfileGpuInitGL();
        MicroProfileSetEnableAllGroups(true);
        MicroProfileSetForceMetaCounters(1);
#endif
    }

    void Engine::run() {
       
        /* Apply config */
        if (mConfig.attachEditor) {
            addSystem<MouseRaySystem>();
            addSystem<EditorSystem>();
            Renderer::addPreProcessShader<SelectableShader>();
            Renderer::addSceneShader<OutlineShader>();
        }

        /* Add engine-specific systems */
        auto& lineShader = Renderer::addSceneShader<LineShader>();
        lineShader.mActive = false;

        util::FrameCounter counter;
        counter.init(glfwGetTime());

        /* Init systems */
        _initSystems();

        /* Initialize new objects and components */
        _processInitQueue();
        Messenger::relayMessages();

        while (!mWindow.shouldClose()) {
            MICROPROFILE_SCOPEI("Engine", "Engine::run", MP_AUTO);

            /* Update frame counter */
            float runTime = static_cast<float>(glfwGetTime());
            counter.update(runTime);

            /* Update display, mouse, keyboard */
            mWindow.update();
            {
                auto& hardware = Engine::createGameObject();
                Engine::addComponent<MouseComponent>(&hardware, mMouse);
                Engine::addComponent<KeyboardComponent>(&hardware, mKeyboard);
                Engine::addComponent<WindowDetailsComponent>(&hardware, mWindow.getDetails());
                Engine::addComponent<FrameStatsComponent>(&hardware, runTime, static_cast<float>(counter.mTimeStep));
                Engine::addComponent<SingleFrameComponent>(&hardware);
            }

            /* Destroy and create objects and components */
            _processKillQueue();
            _processInitQueue();
            Messenger::relayMessages();

            /* Update each system */
            MICROPROFILE_ENTERI("System", "System update", MP_AUTO);
            for (auto& system : mSystems) {
                if (system.second->mActive) {
                    MICROPROFILE_DEFINE(System, "System", system.second->mName.c_str(), MP_AUTO);
		            MICROPROFILE_ENTER(System); 
                    system.second->update(static_cast<float>(counter.mTimeStep));
                    MICROPROFILE_LEAVE();
                }
            }
            Messenger::relayMessages();
            MICROPROFILE_LEAVE();

            /* Update imgui functions */
            if (mImGuiEnabled) {
                MICROPROFILE_ENTERI("Engine", "_runImGui", MP_AUTO);
                ImGui::GetIO().FontGlobalScale = 2.0f;
                _runImGui(counter);
                MICROPROFILE_LEAVE();
                Messenger::relayMessages();
            }

            /* Render */
            // TODO - only run this at 60FPS in its own thread
            // TODO - should this go after processkillqueue?
            Renderer::render(static_cast<float>(counter.mTimeStep), mWindow);
            Messenger::relayMessages();

            for (auto& frameComponent : Engine::getComponents<SingleFrameComponent>()) {
                Engine::removeGameObject(frameComponent->getGameObject());
            }

            MicroProfileFlip(0);
        }

        shutDown();
	    MicroProfileShutdown();
    }

    GameObject & Engine::createGameObject() {
        mGameObjectInitQueue.emplace_back(std::make_unique<GameObject>());
        return *mGameObjectInitQueue.back().get();
    }

    void Engine::removeGameObject(GameObject &go) {
        mGameObjectKillQueue.push_back(&go);
    }

    void Engine::_removeComponent(std::type_index type, Component* component) {
        assert(mComponents.count(type)); // trying to remove a type of component that was never added
        mComponentKillQueue.emplace_back(type, component);
    }

    void Engine::_processInitQueue() {
        MICROPROFILE_SCOPEI("Engine", "_processKillQueue()", MP_AUTO);
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

    void Engine::shutDown() {
        // Clean up GameObjects and components
        for (auto& gameObject : mGameObjects) {
            removeGameObject(*gameObject);
        }
        _processKillQueue();

        // Clean up Renderer
        Renderer::shutDown();

        // Clean up GL objects
        for (auto& mesh : Library::mMeshes) {
            mesh.second->destroy();
        }
        for (auto& texture : Library::mTextures) {
            texture.second->destroy();
        }
        for (auto& frameBuffer : Library::mFramebuffers) {
            frameBuffer.second->destroy();
        }

        mWindow.shutDown();
    }

    void Engine::_processKillQueue() {
        MICROPROFILE_SCOPEI("Engine", "_processKillQueue()", MP_AUTO);
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

    void Engine::_runImGui(const util::FrameCounter& counter) {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Stats")) {
                // Translate FPS to floats
                std::vector<float> FPSfloats;
                std::vector<int> FPSInts = counter.mFPSList;
                FPSfloats.resize(FPSInts.size());
                for (size_t i = 0; i < FPSInts.size(); i++) {
                    FPSfloats[i] = static_cast<float>(FPSInts[i]);
                }
                ImGui::PlotLines("FPS", FPSfloats.data(), static_cast<int>(FPSfloats.size()), 0, std::to_string(counter.mFPS).c_str());
                ImGui::PlotLines("Frame time", counter.mTimeStepList.data(), static_cast<int>(counter.mTimeStepList.size()), 0, std::to_string(counter.mTimeStep * 1000.f).c_str());
                if (ImGui::Button("VSync")) {
                    mWindow.toggleVSync();
                }
                if (auto stats = Engine::getComponentTuple<MouseComponent, WindowDetailsComponent>()) {
                    if (ImGui::TreeNodeEx("Window", ImGuiTreeNodeFlags_DefaultOpen)) {
                        stats->get<WindowDetailsComponent>()->imGuiEditor();
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNodeEx("Mouse", ImGuiTreeNodeFlags_DefaultOpen)) {
                        stats->get<MouseComponent>()->imGuiEditor();
                        ImGui::TreePop();
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("ECS")) {
                ImGui::Text("GameObjects:  %d", getGameObjects().size());
                size_t count = 0;
                for (auto go : getGameObjects()) {
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
                static bool _showBB = false;
                if (ImGui::Checkbox("Show bounding boxes", &_showBB)) {
                    if (_showBB) {
                        Renderer::getShader<LineShader>().mActive = true;
                        for (auto box : Engine::getComponents<BoundingBoxComponent>()) {
                            auto line = box->getGameObject().getComponentByType<LineMeshComponent>();
                            if (!line) {
                                line = &Engine::addComponent<LineMeshComponent>(&box->getGameObject());

                                line->mUseParentSpatial = true;
                                line->mWriteDepth = true;
                                line->mOverrideColor = util::genRandomVec3(0.3f, 1.f);

                                glm::vec3 NearLeftBottom{ box->mMin };
                                glm::vec3 NearLeftTop{ box->mMin.x, box->mMax.y, box->mMin.z };
                                glm::vec3 NearRightBottom{ box->mMax.x, box->mMin.y, box->mMin.z };
                                glm::vec3 NearRightTop{ box->mMax.x, box->mMax.y, box->mMin.z };
                                glm::vec3 FarLeftBottom{ box->mMin.x, box->mMin.y,  box->mMax.z };
                                glm::vec3 FarLeftTop{ box->mMin.x, box->mMax.y,     box->mMax.z };
                                glm::vec3 FarRightBottom{ box->mMax.x, box->mMin.y, box->mMax.z };
                                glm::vec3 FarRightTop{ box->mMax };

                                line->addNode(NearLeftBottom);
                                line->addNode(NearLeftTop);
                                line->addNode(NearRightTop);
                                line->addNode(NearRightBottom);
                                line->addNode(NearLeftBottom);
                                line->addNode(FarLeftBottom);
                                line->addNode(FarLeftTop);
                                line->addNode(NearLeftTop);
                                line->addNode(FarLeftTop);
                                line->addNode(FarRightTop);
                                line->addNode(NearRightTop);
                                line->addNode(FarRightTop);
                                line->addNode(FarRightBottom);
                                line->addNode(NearRightBottom);
                                line->addNode(FarRightBottom);
                                line->addNode(FarLeftBottom);
                            }
                        }
                    }
                    else {
                        for (auto box : Engine::getComponents<BoundingBoxComponent>()) {
                            auto line = box->getGameObject().getComponentByType<LineMeshComponent>();
                            if (line) {
                                Engine::removeComponent(*line);
                            }
                        }
                    }
                }
                if (ImGui::TreeNodeEx("Shaders", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto shadersFunc = [&](std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>>& shaders, const std::string swapName) {
                        for (unsigned i = 0; i < shaders.size(); i++) {
                            auto& shader = shaders[i];
                            ImGui::PushID(i);
                            bool treeActive = ImGui::TreeNodeEx(shader.second->mName.c_str());
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

                    if (Renderer::mComputeShaders.size() && ImGui::TreeNodeEx("Compute", ImGuiTreeNodeFlags_DefaultOpen)) {
                        shadersFunc(Renderer::mComputeShaders, "COMPUTE_SWAP");
                        ImGui::TreePop();
                    }
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
#pragma warning(push)
#pragma warning(disable: 4312)
                ImGui::Image(reinterpret_cast<ImTextureID>(texture.mTextureID), ImVec2(scale * texture.mWidth, scale * texture.mHeight), ImVec2(0, 1), ImVec2(1, 0));
#pragma warning(pop)
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
                        ImGui::Text("%s", m.first.c_str());
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNodeEx("Textures", ImGuiTreeNodeFlags_DefaultOpen)) {
                    for (auto& t : Library::mTextures) {
                        if (ImGui::TreeNode((t.first + " (" + std::to_string(t.second->mTextureID) + ")" + " [" + std::to_string(t.second->mWidth) + ", " + std::to_string(t.second->mHeight) + "]").c_str())) {
                            // TODO - only run this on 2D textures
                            textureFunc(*t.second);
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                ImGui::EndMenu();
            }
            if (mConfig.attachEditor && ImGui::BeginMenu("Editor")) {
                if (auto selected = getSingleComponent<SelectedComponent>()) {
                    if (ImGui::Button("Delete entity")) {
                        Engine::removeGameObject(selected->getGameObject());
                    }
                    auto allComponents = selected->getGameObject().getComponentsMap();
                    static std::optional<std::type_index> type;
                    ImGui::Separator();
                    if (ImGui::BeginCombo("Components", type ? type->name() : "Edit components")) {
                        type = std::nullopt;
                        for (auto comp : allComponents) {
                            if (comp.second.size()) {
                                if (ImGui::Selectable(comp.first.name())) {
                                    type = std::make_optional<std::type_index>(comp.first);
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                    if (type.has_value()) {
                        auto components = allComponents[type.value()];
                        if (components.size()) {
                            static int index = 0;
                            if (components.size() > 1) {
                                ImGui::Indent();
                                ImGui::SliderInt("Index", &index, 0, static_cast<int>(components.size()) - 1);
                                ImGui::Unindent();
                            }
                            ImGui::Indent();
                            components[index]->imGuiEditor();
                            ImGui::Unindent();

                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.81f, 0.20f, 0.20f, 0.40f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.81f, 0.20f, 0.20f, 1.00f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.81f, 0.15f, 0.05f, 1.00f));
                            if (ImGui::Button("Remove Component", ImVec2(ImGui::GetWindowWidth() * 0.9f, 0))) {
                                _removeComponent(type.value(), components[index]);
                                if (components.size() == 1) {
                                    index = 0;
                                    type = std::nullopt;
                                }
                                else if (index == components.size() - 1) {
                                    index--;
                                }
                            }
                            ImGui::PopStyleColor(3);
                        }
                    }
                    /* Attaching new components here would be nice, but there's problems:
                        - There's no reflection that provides a static list of all components possible (including any application-specific components)
                            - They _could_ all be registered manually, both by the Engine for Engine-specific components and by the Application for app-specific components
                            - They would need some dummy GameObject to be tied to
                        - Components have a deleted copy construct
                            - The copy constructor could be made protected, but then every single component would need some clone(GameObject&) function
                              to create a new unique_ptr of itself. That's too much overhead.
                    */
                }
                ImGui::Separator();
                if (ImGui::Button("Create new GameObject")) {
                    removeComponent<SelectedComponent>(*getSingleComponent<SelectedComponent>());
                    auto& go = createGameObject();
                    addComponent<BoundingBoxComponent>(&go, *Library::getMesh("sphere"));
                    addComponent<SpatialComponent>(&go);
                    addComponent<SelectableComponent>(&go);
                    addComponent<SelectedComponent>(&go);
                    addComponent<MeshComponent>(&go, *Library::getMesh("sphere"));
                    addComponent<renderable::WireframeRenderable>(&go);
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
}