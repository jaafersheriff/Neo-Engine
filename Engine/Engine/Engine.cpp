// allows program to be run on dedicated graphics processor for laptops with
// both integrated and dedicated graphics using Nvidia Optimus
#ifdef _WIN32
extern "C" {
	_declspec(dllexport) unsigned int NvOptimusEnablement = 0x00000001;
}
#endif

#include "Engine.hpp"

#include "Renderer/Renderer.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/EngineComponents/SingleFrameComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/CollisionComponent/MouseRayComponent.hpp"
#include "ECS/Component/EngineComponents/DebugBoundingBox.hpp"
#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"
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

#include <ImGuizmo.h>

#include <time.h>
#include <iostream>

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include <GLFW/glfw3.h>

namespace neo {

	/* ECS */
	ECS Engine::mECS;
	MouseRaySystem Engine::mMouseRaySystem;
	SelectingSystem Engine::mSelectingSystem;

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

			/* Init GLEW */
			glewExperimental = GL_FALSE;
			NEO_ASSERT(glewInit()== GLEW_OK, "Failed to init GLEW");
		}
		ServiceLocator<ImGuiManager>::set();
		ServiceLocator<ImGuiManager>::ref().init(mWindow.getWindow());

		Library::init();

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
		TracyGpuContext;

	}

	void Engine::run(DemoWrangler& demos) {

		util::Profiler profiler(mWindow.getDetails().mRefreshRate, mWindow.getDetails().mDPIScale);
		demos.setForceReload();
		
		while (!mWindow.shouldClose()) {
			TRACY_ZONEN("Engine::run");

			{
				TRACY_ZONEN("Frame");
				{
					TRACY_ZONEN("Frame Update");
					if (demos.needsReload()) {
						_swapDemo(demos);
					}

					_startFrame(profiler);
					Messenger::relayMessages(mECS);

					/* Destroy and create objects and components */
					mECS.flush();
					Messenger::relayMessages(mECS);

					{
						TRACY_ZONEN("Demo::update");
						demos.getCurrentDemo()->update(mECS);
					}

					/* Update each system */
					mECS._updateSystems();
					Messenger::relayMessages(mECS);

					/* Update imgui functions */
					if (!mWindow.isMinimized() && ServiceLocator<ImGuiManager>::ref().isEnabled()) {
						TRACY_ZONEN("ImGui");
						ServiceLocator<ImGuiManager>::ref().begin();

						{
							TRACY_ZONEN("Demo Imgui");
							demos.imGuiEditor(mECS);
						}
						mECS.imguiEdtor();
						Library::imGuiEditor();
						ServiceLocator<ImGuiManager>::ref().imGuiEditor();
						ServiceLocator<Renderer>::ref().imGuiEditor(mWindow, mECS);
						profiler.imGuiEditor();

						ServiceLocator<ImGuiManager>::ref().end();
						Messenger::relayMessages(mECS);
					}
				}

				/* Render */
				// TODO - only run this at 60FPS in its own thread
				// TODO - should this go after processkillqueue?
				{
					TRACY_ZONEN("Frame Render");
					if (!mWindow.isMinimized()) {
						ServiceLocator<Renderer>::ref().render(mWindow, demos.getCurrentDemo(), mECS);
					}
					Messenger::relayMessages(mECS);
				}

				_endFrame();
				Messenger::relayMessages(mECS);
			}

			mWindow.flip();
			TracyGpuCollect;
			FrameMark;
		}

		demos.getCurrentDemo()->destroy();
		shutDown();
	}

	void Engine::_swapDemo(DemoWrangler& demos) {
		TRACY_ZONE();

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
		Loader::init(config.resDir, config.shaderDir);
		_createPrefabs();

		demos.getCurrentDemo()->init(mECS);

		/* Init systems */
		mECS._initSystems();

		/* Initialize new objects and components */
		mECS.flush();
		Messenger::relayMessages(mECS);
	}

	void Engine::_createPrefabs() {
		/* Generate basic meshes */
		Library::insertMesh(std::string("cube"), prefabs::generateCube());
		Library::insertMesh(std::string("quad"), prefabs::generateQuad());
		Library::insertMesh(std::string("sphere"), prefabs::generateSphere(2));

		/* Generate basic textures*/
		uint8_t data[] = { 0x00, 0x00, 0x00, 0xFF };
		Library::createTexture("black", {}, glm::u16vec3(1), data);
		data[0] = data[1] = data[2] = 0xFF;
		Library::createTexture("white", {}, glm::u16vec3(1), data);
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

	void Engine::_startFrame(util::Profiler& profiler) {
		TRACY_ZONE();

		/* Update frame counter */
		float runTime = static_cast<float>(glfwGetTime());
		profiler.update(runTime);

		/* Update display, mouse, keyboard */
		mWindow.updateHardware();
		if (!mWindow.isMinimized() && ServiceLocator<ImGuiManager>::ref().isEnabled()) {
			ServiceLocator<ImGuiManager>::ref().update();
		}

		glm::uvec2 viewportSize;
		glm::uvec2 viewportPosition;
			if (ServiceLocator<ImGuiManager>::ref().isEnabled()) {
				viewportSize = ServiceLocator<ImGuiManager>::ref().getViewportSize();
				viewportPosition = mWindow.getDetails().mPos + ServiceLocator<ImGuiManager>::ref().getViewportOffset();
			}
			else {
				viewportSize = mWindow.getDetails().mSize;
				viewportPosition = mWindow.getDetails().mPos;
			}
		{
			TRACY_ZONEN("FrameStats Entity");
			auto hardware = mECS.createEntity();
			mECS.addComponent<MouseComponent>(hardware, mMouse);
			mECS.addComponent<KeyboardComponent>(hardware, mKeyboard);
			mECS.addComponent<ViewportDetailsComponent>(hardware, viewportSize, viewportPosition);
			mECS.addComponent<FrameStatsComponent>(hardware, runTime, static_cast<float>(profiler.mTimeStep));
			mECS.addComponent<SingleFrameComponent>(hardware);
		}

		{
			TRACY_ZONEN("Aspect Ratio");
			for (auto& entity : mECS.getView<MainCameraComponent>()) {
				mECS.getComponent<PerspectiveCameraComponent>(entity)->setAspectRatio(viewportSize.x / static_cast<float>(viewportSize.y));
			}
		}
		{
			TRACY_ZONEN("Selecting");
			if (!ImGuizmo::IsUsing()) {
				mMouseRaySystem.update(mECS);
				mSelectingSystem.update(mECS);
			}
		}
	}

	void Engine::_endFrame() {
		TRACY_ZONE();

		for(auto& entity : mECS.getView<SingleFrameComponent>()) {
			mECS.removeEntity(entity);
		}

		// Flush resources
		Library::tick();
	}
}
