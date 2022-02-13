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

#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/EngineComponents/SingleFrameComponent.hpp"
#include "ECS/Component/HardwareComponent/KeyboardComponent.hpp"
#include "ECS/Component/HardwareComponent/WindowDetailsComponent.hpp"
#include "ECS/Systems/SelectingSystems/MouseRaySystem.hpp"
#include "ECS/Systems/SelectingSystems/EditorSystem.hpp"

#include "Hardware/WindowSurface.hpp"
#include "Hardware/Keyboard.hpp"
#include "Hardware/Mouse.hpp"

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
    ECS Engine::mECS;

    /* ImGui */
    bool Engine::mImGuiEnabled = true;
    std::unordered_map<std::string, Engine::ImGuiFunc> Engine::mImGuiFuncs;

    /* Hardware */
    WindowSurface Engine::mWindow;
    Keyboard Engine::mKeyboard;
    Mouse Engine::mMouse;

    ECS& Engine::init(EngineConfig config) {

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

        return mECS;
    }

    void Engine::run(std::vector<neo::IDemo*>& demos, uint32_t currDemo) {
        demos[currDemo]->init(mECS);
       
        /* Apply config */
        if (mConfig.attachEditor) {
            mECS.addSystem<MouseRaySystem>();
            mECS.addSystem<EditorSystem>();
            Renderer::addPreProcessShader<SelectableShader>();
            Renderer::addSceneShader<OutlineShader>();
        }

        /* Add engine-specific systems */
        auto& lineShader = Renderer::addSceneShader<LineShader>();
        lineShader.mActive = false;

        util::FrameCounter counter;
        counter.init(glfwGetTime());

        /* Init systems */
        mECS._initSystems();

        /* Initialize new objects and components */
        mECS._processInitQueue();
        Messenger::relayMessages(mECS);

        while (!mWindow.shouldClose()) {
            MICROPROFILE_SCOPEI("Engine", "Engine::run", MP_AUTO);

            /* Update frame counter */
            float runTime = static_cast<float>(glfwGetTime());
            counter.update(runTime);

            /* Update display, mouse, keyboard */
            mWindow.update();
            {
                auto& hardware = mECS.createGameObject();
                mECS.addComponent<MouseComponent>(&hardware, mMouse);
                mECS.addComponent<KeyboardComponent>(&hardware, mKeyboard);
                mECS.addComponent<WindowDetailsComponent>(&hardware, mWindow.getDetails());
                mECS.addComponent<FrameStatsComponent>(&hardware, runTime, static_cast<float>(counter.mTimeStep));
                mECS.addComponent<SingleFrameComponent>(&hardware);
            }

            demos[currDemo]->update(mECS);

            /* Destroy and create objects and components */
            mECS._processKillQueue();
            mECS._processInitQueue();
            Messenger::relayMessages(mECS);

            /* Update each system */
            mECS._updateSystems();
            Messenger::relayMessages(mECS);

            /* Update imgui functions */
            if (mImGuiEnabled) {
                MICROPROFILE_ENTERI("Engine", "_runImGui", MP_AUTO);
                ImGui::GetIO().FontGlobalScale = 2.0f;
                _runImGui(counter);
                MICROPROFILE_LEAVE();
                Messenger::relayMessages(mECS);
            }

            /* Render */
            // TODO - only run this at 60FPS in its own thread
            // TODO - should this go after processkillqueue?
            Renderer::render(static_cast<float>(counter.mTimeStep), mWindow, mECS);
            Messenger::relayMessages(mECS);

            for (auto& frameComponent : mECS.getComponents<SingleFrameComponent>()) {
                mECS.removeGameObject(frameComponent->getGameObject());
            }

            MicroProfileFlip(0);
        }

        demos[currDemo]->destroy();
        shutDown();
	    MicroProfileShutdown();
    }

    void Engine::shutDown() {
        mECS.shutDown();

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

        Renderer::shutDown();

        mWindow.shutDown();
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
                mECS._imguiEdtor();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Renderer")) {
                Renderer::_imguiEditor(mECS);
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
                if (auto selected = mECS.getSingleComponent<SelectedComponent>()) {
                    if (ImGui::Button("Delete entity")) {
                        mECS.removeGameObject(selected->getGameObject());
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
                                mECS._removeComponent(type.value(), components[index]);
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
                    mECS.removeComponent<SelectedComponent>(*mECS.getSingleComponent<SelectedComponent>());
                    auto& go = mECS.createGameObject();
                    mECS.addComponent<BoundingBoxComponent>(&go, *Library::getMesh("sphere"));
                    mECS.addComponent<SpatialComponent>(&go);
                    mECS.addComponent<SelectableComponent>(&go);
                    mECS.addComponent<SelectedComponent>(&go);
                    mECS.addComponent<MeshComponent>(&go, *Library::getMesh("sphere"));
                    mECS.addComponent<renderable::WireframeRenderable>(&go);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(mConfig.APP_NAME.c_str())) {
                for (auto & it : mImGuiFuncs) {
                    if (ImGui::TreeNodeEx(it.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                        it.second(mECS);
                        ImGui::TreePop();
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}