// allows program to be run on dedicated graphics processor for laptops with
// both integrated and dedicated graphics using Nvidia Optimus
#ifdef _WIN32
extern "C" {
    _declspec(dllexport) unsigned int NvOptimusEnablement = 0x00000001;
}
#endif

#include "Engine.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/Shader/OutlineShader.hpp"
#include "Renderer/Shader/WireframeShader.hpp"
#include "Renderer/Shader/LineShader.hpp"

#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/EngineComponents/SingleFrameComponent.hpp"
#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/HardwareComponent/KeyboardComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"

#include "ImGuiManager.hpp"

#include "Hardware/WindowSurface.hpp"
#include "Hardware/Keyboard.hpp"
#include "Hardware/Mouse.hpp"

#include "Messaging/Messenger.hpp"

#include "Loader/Loader.hpp"
#include "Loader/MeshGenerator.hpp"

#include "Util/Profiler.hpp"
#include "Util/Log/Log.hpp"
#include "Util/ServiceLocator.hpp"

#include <time.h>
#include <iostream>

#include <tracy/Tracy.hpp>

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

#if MICROPROFILE_ENABLED
        NEO_LOG_I("Microprofile enabled");
        MicroProfileOnThreadCreate("MAIN THREAD");
        MicroProfileGpuInitGL();
        MicroProfileSetEnableAllGroups(true);
        MicroProfileSetForceMetaCounters(1);
#endif

        ServiceLocator<Renderer>::ref().init();
        {
            auto& details = ServiceLocator<Renderer>::ref().mDetails;
            /* Set max work group */
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &details.mMaxComputeWorkGroupSize.x);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &details.mMaxComputeWorkGroupSize.y);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &details.mMaxComputeWorkGroupSize.z);
            char buf[512];
            sprintf(buf, "%s", glGetString(GL_VENDOR));
            details.mVendor = buf;
            sprintf(buf, "%s", glGetString(GL_RENDERER));
            details.mRenderer = buf;
            sprintf(buf, "%s", glGetString(GL_SHADING_LANGUAGE_VERSION));
            details.mShadingLanguage = buf;

            int bytes = sprintf(buf, "OpenGL Version: %d.%d", details.mGLMajorVersion, details.mGLMinorVersion);;
            TracyAppInfo(buf, bytes);
            bytes = sprintf(buf, "Max Shading Language:  %s", details.mShadingLanguage.c_str());
            TracyAppInfo(buf, bytes);
            bytes = sprintf(buf, "Used Shading Language: %s", details.mGLSLVersion.c_str());
            TracyAppInfo(buf, bytes);
            bytes = sprintf(buf, "Vendor: %s", details.mVendor.c_str());
            TracyAppInfo(buf, bytes);
            bytes = sprintf(buf, "Renderer: %s", details.mRenderer.c_str());
            TracyAppInfo(buf, bytes);
            bytes = sprintf(buf, "Max Compute Work Group Size: [%d, %d, %d]", details.mMaxComputeWorkGroupSize.x, details.mMaxComputeWorkGroupSize.y, details.mMaxComputeWorkGroupSize.z);
            TracyAppInfo(buf, bytes);

        }
    }

    void Engine::run(DemoWrangler& demos) {

        util::Profiler profiler;
        demos.setForceReload();
        
        while (!mWindow.shouldClose() && !mKeyboard.isKeyPressed(GLFW_KEY_ESCAPE)) {
            ZoneScoped;

            if (demos.needsReload()) {
                _swapDemo(demos);
            }

            /* Update frame counter */
            float runTime = static_cast<float>(glfwGetTime());
            profiler.update(runTime);

            /* Update display, mouse, keyboard */
            mWindow.updateHardware();
            ServiceLocator<ImGuiManager>::ref().update();
            Messenger::relayMessages(mECS);

            {
                ZoneScopedN("FrameStats Entity");
                auto hardware = mECS.createEntity();
                mECS.addComponent<MouseComponent>(hardware, mMouse);
                mECS.addComponent<KeyboardComponent>(hardware, mKeyboard);
                if (ServiceLocator<ImGuiManager>::ref().isEnabled()) {
                    mECS.addComponent<ViewportDetailsComponent>(hardware, ServiceLocator<ImGuiManager>::ref().getViewportSize(), mWindow.getDetails().mPos + ServiceLocator<ImGuiManager>::ref().getViewportOffset());
                }
                else {
                    mECS.addComponent<ViewportDetailsComponent>(hardware, mWindow.getDetails().mSize, mWindow.getDetails().mPos);
                }
                mECS.addComponent<FrameStatsComponent>(hardware, runTime, static_cast<float>(profiler.mTimeStep));
                mECS.addComponent<SingleFrameComponent>(hardware);
            }

            {
                ZoneScopedN("Demo::update");
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
                    ZoneScopedN("ImGui Calls");
                    ServiceLocator<ImGuiManager>::ref().begin();

                    {
                        ZoneScopedN("demos.imGuiEditor");
                        demos.imGuiEditor(mECS);
                    }
                    {
                        ZoneScopedN("mECS.imguiEdtor");
                        mECS.imguiEdtor();
                    }
                    {
                        ZoneScopedN("Renderer.imGuiEditor");
                        ServiceLocator<Renderer>::ref().imGuiEditor(mWindow, mECS);
                    }
                    {
                        ZoneScopedN("Library::imGuiEditor");
                        Library::imGuiEditor();
                    }
                    {
                        ZoneScopedN("ImGuiManager.imGuiEditor");
                        ServiceLocator<ImGuiManager>::ref().imGuiEditor();
                    }
                    {
                        ZoneScopedN("ImGuiManager.imGuiEditor");
                        profiler.imGuiEditor();
                    }

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

            FrameMark;
        }

        demos.getCurrentDemo()->destroy();
        shutDown();
    }

    void Engine::_swapDemo(DemoWrangler& demos) {
        ZoneScopedN("_swapDemo");

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
        mECS.clean();
        Messenger::clean();
        Library::clean();
        ServiceLocator<Renderer>::ref().clean();
        ServiceLocator<Renderer>::reset();
        ServiceLocator<ImGuiManager>::ref().destroy();
        mWindow.shutDown();
    }
}
