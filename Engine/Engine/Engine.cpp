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
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Systems/SelectingSystems/MouseRaySystem.hpp"
#include "ECS/Systems/SelectingSystems/EditorSystem.hpp"

#include "ImGuiManager.hpp"

#include "Hardware/WindowSurface.hpp"
#include "Hardware/Keyboard.hpp"
#include "Hardware/Mouse.hpp"

#include "ECS/Messaging/Messenger.hpp"

#include "Loader/Loader.hpp"
#include "Loader/MeshGenerator.hpp"

#include "Util/FrameCounter.hpp"
#include "Util/Log/Log.hpp"
#include "Util/ServiceLocator.hpp"

#include <time.h>
#include <iostream>
#include <microprofile.h>

namespace neo {

    /* ECS */
    ECS Engine::mECS;

    /* Hardware */
    WindowSurface Engine::mWindow;
    Keyboard Engine::mKeyboard;
    Mouse Engine::mMouse;

    void Engine::init() {

        srand((unsigned int)(time(0)));

        ServiceLocator<Renderer>::set(4, 4);

        {
            NEO_ASSERT(mWindow.init("") == 0, "Failed initializing Window");
            GLFWimage icons[1];
            int components;
            uint8_t* data = Loader::_loadTextureData(icons[0].width, icons[0].height, components, "icon.png", {}, false);
            if (data) {
                icons[0].pixels = data;
                glfwSetWindowIcon(mWindow.getWindow(), 1, icons);
            }
            Loader::_cleanTextureData(data);
        }
        ServiceLocator<ImGuiManager>::set();
        ServiceLocator<ImGuiManager>::ref().init(mWindow.getWindow());

        mKeyboard.init();
        mMouse.init();
#if MICROPROFILE_ENABLED
        NEO_LOG_I("Microprofile enabled");
        MicroProfileOnThreadCreate("MAIN THREAD");
        MicroProfileGpuInitGL();
        MicroProfileSetEnableAllGroups(true);
        MicroProfileSetForceMetaCounters(1);
#endif

        ServiceLocator<Renderer>::ref().init();
    }

    void Engine::run(DemoWrangler& demos) {

        util::FrameCounter counter;
        demos.setForceReload();
        
        while (!mWindow.shouldClose() && !mKeyboard.isKeyPressed(GLFW_KEY_ESCAPE)) {
            MICROPROFILE_SCOPEI("Engine", "Engine::run", MP_AUTO);

            if (demos.needsReload()) {
                _swapDemo(demos);
            }

            /* Update frame counter */
            float runTime = static_cast<float>(glfwGetTime());
            counter.update(runTime);

            /* Update display, mouse, keyboard */
            mWindow.updateHardware();
            ServiceLocator<ImGuiManager>::ref().update();
            Messenger::relayMessages(mECS);

            {
                MICROPROFILE_SCOPEI("Engine", "FrameStats Entity", MP_AUTO);
                auto hardware = mECS.createEntity();
                mECS.addComponent<MouseComponent>(hardware, mMouse);
                mECS.addComponent<KeyboardComponent>(hardware, mKeyboard);
                if (ServiceLocator<ImGuiManager>::ref().isEnabled()) {
                    mECS.addComponent<ViewportDetailsComponent>(hardware, ServiceLocator<ImGuiManager>::ref().getViewportSize(), mWindow.getDetails().mPos + ServiceLocator<ImGuiManager>::ref().getViewportOffset());
                }
                else {
                    mECS.addComponent<ViewportDetailsComponent>(hardware, mWindow.getDetails().mSize, mWindow.getDetails().mPos);
                }
                mECS.addComponent<FrameStatsComponent>(hardware, runTime, static_cast<float>(counter.mTimeStep));
                mECS.addComponent<SingleFrameComponent>(hardware);
            }

            {
                MICROPROFILE_SCOPEI("Engine", "Demo::update", MP_AUTO);
                demos.getCurrentDemo()->update(mECS);
            }

            /* Destroy and create objects and components */
            mECS.flush();
            Messenger::relayMessages(mECS);

            if (!mWindow.isMinimized()) {
                /* Update each system */
                mECS._updateSystems();
                Messenger::relayMessages(mECS);

                /* Update imgui functions */
                if (ServiceLocator<ImGuiManager>::ref().isEnabled()) {
                    MICROPROFILE_SCOPEI("ImGui", "ImGui", MP_AUTO);
                    ServiceLocator<ImGuiManager>::ref().begin();

                    {
                        MICROPROFILE_SCOPEI("ImGui", "demos.imGuiEditor", MP_AUTO);
                        demos.imGuiEditor(mECS);
                    }
                    {
                        MICROPROFILE_SCOPEI("ImGui", "mECS.imguiEdtor", MP_AUTO);
                        mECS.imguiEdtor();
                    }
                    {
                        MICROPROFILE_SCOPEI("ImGui", "Renderer.imGuiEditor", MP_AUTO);
                        ServiceLocator<Renderer>::ref().imGuiEditor(mWindow, mECS);
                    }
                    {
                        MICROPROFILE_SCOPEI("ImGui", "Library::imGuiEditor", MP_AUTO);
                        Library::imGuiEditor();
                    }
                    {
                        MICROPROFILE_SCOPEI("ImGui", "ImGuiManager.imGuiEditor", MP_AUTO);
                        ServiceLocator<ImGuiManager>::ref().imGuiEditor();
                    }
                    imGuiEditor(demos, counter);

                    ServiceLocator<ImGuiManager>::ref().end();
                }
                Messenger::relayMessages(mECS);
            }

            /* Render */
            // TODO - only run this at 60FPS in its own thread
            // TODO - should this go after processkillqueue?
            ServiceLocator<Renderer>::ref().render(mWindow, mECS);
            Messenger::relayMessages(mECS);

            // TODO - this should be its own system
            mECS.getView<SingleFrameComponent>().each([](ECS::Entity entity, SingleFrameComponent&) {
                mECS.removeEntity(entity);
            });

            MicroProfileFlip(0);
        }

        demos.getCurrentDemo()->destroy();
        shutDown();
	    MicroProfileShutdown();
    }

    void Engine::_swapDemo(DemoWrangler& demos) {
        MICROPROFILE_SCOPEI("Engine", "_swapDemo", MP_AUTO);

        /* Destry the old state*/
        demos.getCurrentDemo()->destroy();
        mECS.clean();
        Library::clean();
        ServiceLocator<Renderer>::ref().clean();
        Messenger::clean();

        /* Init the new state */
        ServiceLocator<ImGuiManager>::ref().reset();
        demos.swap();
        auto config = demos.getConfig();
        mWindow.reset(config.name);
        mMouse.init();
        mKeyboard.init();
        ServiceLocator<Renderer>::ref().setDemoConfig(config);
        ServiceLocator<Renderer>::ref().init();
        Loader::init(config.resDir);
        _createPrefabs();

        /* Apply config */
        if (config.attachEditor) {
            NEO_LOG("Attaching editor");
            mECS.addSystem<MouseRaySystem>();
            mECS.addSystem<EditorSystem>();
            ServiceLocator<Renderer>::ref().addPreProcessShader<SelectableShader>();
            ServiceLocator<Renderer>::ref().addSceneShader<OutlineShader>();
            ServiceLocator<Renderer>::ref().addSceneShader<WireframeShader>();
        }

        /* Add engine-specific systems */
        auto& lineShader = ServiceLocator<Renderer>::ref().addSceneShader<LineShader>();
        lineShader.mActive = false;

        demos.getCurrentDemo()->init(mECS, ServiceLocator<Renderer>::ref());

        /* Init systems */
        mECS._initSystems();

        /* Initialize new objects and components */
        mECS.flush();
        Messenger::relayMessages(mECS);
    }

    void Engine::_createPrefabs() {
        /* Generate basic meshes */
        {
            MeshData meshData;
            prefabs::generateCube(meshData);
            Library::insertMesh(std::string("cube"), meshData);
        }
        {
            MeshData meshData;
            prefabs::generateQuad(meshData);
            Library::insertMesh(std::string("quad"), meshData);
        }
        {
            MeshData meshData;
            prefabs::generateSphere(meshData, 2);
            Library::insertMesh(std::string("sphere"), meshData);
        }

        /* Generate basic textures*/
        uint8_t data[] = { 0x00, 0x00, 0x00, 0xFF };
        auto tex = Library::createEmptyTexture<Texture2D>("black", {});
        tex->update(glm::uvec2(1), data);
        data[0] = data[1] = data[2] = 0xFF;
        tex = Library::createEmptyTexture<Texture2D>("white", {});
        tex->update(glm::uvec2(1), data);
    }

    void Engine::shutDown() {
        NEO_LOG_I("Shutting down...");
        Messenger::clean();
        mECS.clean();
        Library::clean();
        ServiceLocator<Renderer>::ref().clean();
        ServiceLocator<ImGuiManager>::ref().destroy();
        mWindow.shutDown();
    }

    void Engine::imGuiEditor(DemoWrangler& demos, const util::FrameCounter& counter) {
        {
            ImGui::Begin("Stats");
            counter.imGuiEditor();
            ImGui::TextWrapped("Num Draws: %d", ServiceLocator<Renderer>::ref().mStats.mNumDraws);
            ImGui::TextWrapped("Num Shaders: %d", ServiceLocator<Renderer>::ref().mStats.mNumShaders);
            for (auto&& [entity, mouse, viewport] : mECS.getView<MouseComponent, ViewportDetailsComponent>().each()) {
                if (ImGui::TreeNodeEx("Window", ImGuiTreeNodeFlags_DefaultOpen)) {
                    viewport.imGuiEditor();
                    ImGui::TreePop();
                }
                if (ImGui::TreeNodeEx("Mouse", ImGuiTreeNodeFlags_DefaultOpen)) {
                    mouse.imGuiEditor();
                    ImGui::TreePop();
                }
            }
            ImGui::End();
        }

        if (demos.getConfig().attachEditor) {
            ImGui::Begin("Editor");
            if (auto selectedView = mECS.getComponent<SelectedComponent>()) {
                auto&& [selectedEntity, _] = *selectedView;
                if (ImGui::Button("Delete entity")) {
                    mECS.removeEntity(selectedEntity);
                }
                NEO_FAIL("This won't work");
                // auto allComponents = selected->getGameObject().getComponentsMap();
                // static std::optional<std::type_index> type;
                // ImGui::Separator();
                // if (ImGui::BeginCombo("Components", type ? type->name() : "Edit components")) {
                //     type = std::nullopt;
                //     for (auto comp : allComponents) {
                //         if (comp.second.size()) {
                //             if (ImGui::Selectable(comp.first.name())) {
                //                 type = std::make_optional<std::type_index>(comp.first);
                //             }
                //         }
                //     }
                //     ImGui::EndCombo();
                // }
                // if (type.has_value()) {
                //     auto components = allComponents[type.value()];
                //     if (components.size()) {
                //         static int index = 0;
                //         if (components.size() > 1) {
                //             ImGui::Indent();
                //             ImGui::SliderInt("Index", &index, 0, static_cast<int>(components.size()) - 1);
                //             ImGui::Unindent();
                //         }
                //         ImGui::Indent();
                //         components[index]->imGuiEditor();
                //         ImGui::Unindent();

                //         ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.81f, 0.20f, 0.20f, 0.40f));
                //         ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.81f, 0.20f, 0.20f, 1.00f));
                //         ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.81f, 0.15f, 0.05f, 1.00f));
                //         if (ImGui::Button("Remove Component", ImVec2(ImGui::GetWindowWidth() * 0.9f, 0))) {
                //             mECS._removeComponent(type.value(), components[index]);
                //             if (components.size() == 1) {
                //                 index = 0;
                //                 type = std::nullopt;
                //             }
                //             else if (index == components.size() - 1) {
                //                 index--;
                //             }
                //         }
                //         ImGui::PopStyleColor(3);
                //     }
                // }
                /* Attaching new components here would be nice, but there's problems:
                    - There's no reflection that provides a static list of all components possible (including any application-specific components)
                        - They _could_ all be registered manually, both by the Engine for Engine-specific components and by the Application for app-specific components
                */
            }
            ImGui::Separator();
            if (ImGui::Button("Create new GameObject")) {
                {
                    auto view = mECS.getView<SelectedComponent>();
                    NEO_ASSERT(view.size() <= 1, "How are there two items selected at the same time");
                    view.each([](ECS::Entity entity, SelectedComponent&) {
                        mECS.removeComponent<SelectedComponent>(entity);
                    });
                }
                auto go = mECS.createEntity();
                auto sphereMesh = Library::getMesh("sphere");
                mECS.addComponent<BoundingBoxComponent>(go, sphereMesh);
                mECS.addComponent<SpatialComponent>(go);
                mECS.addComponent<SelectableComponent>(go);
                mECS.addComponent<SelectedComponent>(go);
                mECS.addComponent<MeshComponent>(go, sphereMesh.mMesh);
                mECS.addComponent<renderable::WireframeRenderable>(go);
            }
            ImGui::End();
        }
    }
}
