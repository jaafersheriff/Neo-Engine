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
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/EngineComponents/SingleFrameComponent.hpp"
#include "ECS/Component/EngineComponents/AsyncJobComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/DebugBoundingBox.hpp"
#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"
#include "ECS/Component/RenderingComponent/ImGuiDrawComponent.hpp"
#include "ECS/Component/RenderingComponent/RendererParamsComponent.hpp"
#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/HardwareComponent/KeyboardComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"

#include "ImGuiManager.hpp"

#include "Messaging/Messenger.hpp"

#include "Loader/Loader.hpp"
#include "Loader/STBIImageData.hpp"
#include "Loader/MeshGenerator.hpp"
#include "ResourceManager/ResourceManagers.hpp"

#include "Util/Profiler.hpp"
#include "Util/Log/Log.hpp"
#include "Util/ServiceLocator.hpp"

#include <ImGuizmo.h>

#include <time.h>
#include <iostream>

#include <GLFW/glfw3.h>

namespace neo {

	void Engine::init() {

		srand((unsigned int)(time(0)));

		ServiceLocator<Renderer>::set(4, 4);

		{
			NEO_ASSERT(mWindow.init("", ServiceLocator<Renderer>::ref().getDetails()) == 0, "Failed initializing Window");

			std::string path = Loader::ENGINE_RES_DIR + std::string("icon.png");
			STBImageData image(path.c_str(), types::texture::BaseFormats::RGBA, types::ByteFormats::UnsignedByte, false);
			if (image) {
				GLFWimage icons[1];
				icons[0].pixels = image.mData;
				icons[0].width = image.mWidth;
				icons[0].height = image.mHeight;
				glfwSetWindowIcon(mWindow.getWindow(), 1, icons);
			}

		}

		mRenderThread.start();
		mRenderThread.runSync([](void* context) {
			ServiceLocator<Renderer>::ref().initGPUContext(*static_cast<WindowSurface*>(context));
		}, &mWindow);

		ServiceLocator<ImGuiManager>::set();
		ServiceLocator<ImGuiManager>::ref().init(mWindow);
	}

	void Engine::run(DemoWrangler&& demos) {

		util::Profiler profiler(mWindow.getDetails().mRefreshRate);

		ECS ecs;
		ResourceManagers resourceManagers;

		demos.setForceReload();

		while (!mWindow.shouldClose()) {
			TRACY_ZONEN("Engine::run");

			{
				TRACY_ZONEN("Frame");
				{
					TRACY_ZONEN("Frame Update");
					profiler.begin(glfwGetTime());
					if (demos.needsReload()) {
						if (ecs.mRegistry.storage<AsyncJobComponent>().empty()) {
							_swapDemo(demos, ecs, resourceManagers);
						}
						else {
							NEO_LOG_V("Waiting for async jobs to complete...");
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
						}
					}

					_startFrame(profiler, ecs, resourceManagers);
					Messenger::relayMessages(ecs);

					/* Destroy and create objects and components */
					ecs._flush();
					Messenger::relayMessages(ecs);

					{
						TRACY_ZONEN("Demo::update");
						demos.getCurrentDemo()->update(ecs, resourceManagers);
					}

					/* Update each system */
					ecs._updateSystems(resourceManagers);
					Messenger::relayMessages(ecs);

					/* Update imgui functions */
					if (!mWindow.isMinimized() && ServiceLocator<ImGuiManager>::ref().isEnabled()) {
						TRACY_ZONEN("ImGui");
						ServiceLocator<ImGuiManager>::ref().begin();

						{
							TRACY_ZONEN("Demo Imgui");
							demos.imGuiEditor(ecs, resourceManagers);
						}
						ecs._imguiEdtor();
						resourceManagers._imguiEditor();
						ServiceLocator<ImGuiManager>::ref().imGuiEditor();
						ServiceLocator<Renderer>::ref()._imGuiEditor(mWindow, ecs, resourceManagers);
						profiler.imGuiEditor();
						{
							// TODO - move to its own function hehe
							TRACY_ZONEN("Engine ImGui");
							ImGui::Begin("Engine");
							if (ImGui::TreeNodeEx("Async Jobs", ImGuiTreeNodeFlags_DefaultOpen)) {
								glm::vec3 warningColor = util::sLogSeverityData.at(util::LogSeverity::Warning).second;
								ImVec4 imguiColor(warningColor.x, warningColor.y, warningColor.z, 1.f);
								for (auto&& [_, __, tag] : ecs.getView<AsyncJobComponent, TagComponent>().each()) {
									ImGui::TextColored(imguiColor, "%s", tag.mTag.c_str());
								}
								ImGui::TreePop();
							}
							if (auto hardwareDetails = ecs.getSingleView<MouseComponent, ViewportDetailsComponent>()) {
								auto&& [entity, mouse, viewport] = hardwareDetails.value();
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

						ServiceLocator<ImGuiManager>::ref().end();
						Messenger::relayMessages(ecs);
					}
				}

				{
					TRACY_ZONEN("Frame Render");

					if (!mWindow.isMinimized()) {
						TRACY_ZONEN("Prepare ImGui draw data");
						if (ServiceLocator<ImGuiManager>::ref().isEnabled()) {
							 // Current frame data gets generated now, then flushed and drawn next frame
							ServiceLocator<ImGuiManager>::ref().resolveDrawData(ecs, resourceManagers);

							 // Last frame's data is about to be drawn, queue deletion now and it'll be flushed next frame
							TRACY_ZONEN("Remove draw data");
							for (auto entity : ecs.getView<ImGuiComponent, ImGuiDrawComponent>()) {
								ecs.removeEntity(entity);
							}
						}
						Messenger::relayMessages(ecs);
					}
					{
						// TODO - this shouldnt be in Frame Render, and probably shouldnt be guarded by isMinimzed, but it should be after imgui component resolve
						_endFrame(profiler, ecs);
						Messenger::relayMessages(ecs);

						// Fill the clone
						ECS& renderECS = mRenderECS[mRenderECSIndex];
						mRenderECSIndex ^= 1;
						{
							TRACY_ZONEN("Clone ECS");
							ecs._cloneInto(renderECS);
						}

						// Wait for previous frame to complete
						mRenderThread.wait();

						struct RenderFrameJob {
							ECS& mRenderECS;
							IDemo& mDemo;
							WindowSurface& mWindow;
							util::Profiler& mProfiler;
							ResourceManagers& mResourceManagers;
						};

						mRenderThread.dispatch([](void* context) {
							const std::unique_ptr<RenderFrameJob> job(static_cast<RenderFrameJob*>(context));

							job->mResourceManagers._tick();

							TRACY_GPUN("Frame Render");
							ServiceLocator<Renderer>::ref().render(job->mWindow, &job->mDemo, job->mProfiler, job->mRenderECS, job->mResourceManagers);

							job->mWindow.flip();
							TracyGpuCollect;
						}, new RenderFrameJob{ renderECS, *demos.getCurrentDemo(), mWindow, profiler, resourceManagers });

						Messenger::relayMessages(ecs);
					}
				}

			}

			FrameMark;
			profiler.end(glfwGetTime());
		}

		// A frame is still in flight now that the wait moved - drain it before tearing anything down.
		mRenderThread.wait();

		demos.getCurrentDemo()->destroy();
		mRenderThread.runSync([](void* context) {
			static_cast<ResourceManagers*>(context)->_clear();
			ServiceLocator<Renderer>::ref().clean();
		}, &resourceManagers);
		mRenderThread.stop();
		shutDown(ecs, resourceManagers);
	}

	void Engine::_swapDemo(DemoWrangler& demos, ECS& ecs, ResourceManagers& resourceManagers) {
		TRACY_ZONE();

		mRenderThread.wait();

		/* Destroy the old state */
		demos.getCurrentDemo()->destroy();
		mRenderThread.runSync([](void* context) {
			static_cast<ResourceManagers*>(context)->_clear();
			ServiceLocator<Renderer>::ref().clean();
		}, &resourceManagers);
		ecs._clean();
		for (auto& renderECS : mRenderECS) {
			renderECS._clean();
		}
		Messenger::clean();

		/* Init the new state */
		ServiceLocator<ImGuiManager>::ref().reset();
		demos.swap();
		auto config = demos.getConfig();
		mWindow.reset(config.name);
		mMouse.init();
		mKeyboard.init();
		ServiceLocator<Renderer>::ref().setDemoConfig(config);
		Loader::init(config.resDir, config.shaderDir);
		_createPrefabs(resourceManagers);
		ServiceLocator<ImGuiManager>::ref().reload(resourceManagers);

		mRenderThread.runSync([](void* context) {
			ResourceManagers& managers = *static_cast<ResourceManagers*>(context);
			managers._init();
			ServiceLocator<Renderer>::ref().init();
			managers._tick();
		}, &resourceManagers);

		/* Init 'singleton' entities/components */
		ecs.submitEntity(std::move(ECS::EntityBuilder{}.attachComponent<RendererParamsComponent>()));
		demos.getCurrentDemo()->init(ecs, resourceManagers);

		/* Init systems */
		ecs._initSystems();

		/* Initialize new objects and components */
		ecs._flush();
		Messenger::relayMessages(ecs);
	}

	void Engine::_createPrefabs(ResourceManagers& resourceManagers) {
		/* Generate basic meshes */
		auto loadMesh = [&](HashedString name, MeshLoadDetails& details) {
			auto id = resourceManagers.mMeshManager.asyncLoad(name, details);
			for (auto&& [type, buffer] : details.mVertexBuffers) {
				delete[] buffer.mData;
			}
			if (details.mElementBuffer) {
				delete[] details.mElementBuffer->mData;
			}
			return id;
		};
		loadMesh("cube", *prefabs::generateCube());
		loadMesh("quad", *prefabs::generateQuad());
		loadMesh("sphere", *prefabs::generateSphere(2));
		loadMesh("icosahedron", *prefabs::generateIcosahedron());
		loadMesh("tetrahedron", *prefabs::generateTetrahedron());

		/* Generate basic textures*/
		TextureBuilder builder;
		builder.mDimensions.x = 1;
		builder.mDimensions.y = 1;

		uint8_t data[] = { 0x00, 0x00, 0x00, 0xFF };
		{
			builder.mData = data;
			auto id = resourceManagers.mTextureManager.asyncLoad(HashedString("black"), builder);
			NEO_UNUSED(id);
		}
		{
			data[0] = data[1] = data[2] = 0xFF;
			builder.mData = data;
			auto id = resourceManagers.mTextureManager.asyncLoad(HashedString("white"), builder);
			NEO_UNUSED(id);
		}
	}

	void Engine::shutDown(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_LOG_I("Shutting down...");
		ecs._clean();
		Messenger::clean();
		NEO_UNUSED(resourceManagers);
		ServiceLocator<Renderer>::reset();
		ServiceLocator<ImGuiManager>::ref().destroy();
		mWindow.shutDown();
	}

	void Engine::_startFrame(util::Profiler& profiler, ECS& ecs, ResourceManagers& resourceManagers) {
		TRACY_ZONE();

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
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<MouseComponent>(mMouse)
				.attachComponent<KeyboardComponent>(mKeyboard)
				.attachComponent<ViewportDetailsComponent>(viewportSize, viewportPosition)
				.attachComponent<FrameStatsComponent>(static_cast<float>(profiler.getRunTime()), static_cast<float>(profiler.getDeltaTime()))
				.attachComponent<SingleFrameComponent>()
			));
		}

		{
			TRACY_ZONEN("Aspect Ratio");
			auto cameraTuple = ecs.getSingleView<MainCameraComponent, CameraComponent>();
			if (cameraTuple) {
				auto& [_, __, camera] = *cameraTuple;
				if (camera.getType() == CameraComponent::CameraType::Perspective) {
					camera.setPerspective(CameraComponent::Perspective{
						camera.getPerspective().mFOV,
						viewportSize.x / static_cast<float>(viewportSize.y)
						});
				}
			}
		}
		{
			TRACY_ZONEN("Selecting");
			if (!ImGuizmo::IsUsing()) {
				mMouseRaySystem.update(ecs, resourceManagers);
				mSelectingSystem.update(ecs, resourceManagers);
			}
		}
		{
			TRACY_ZONEN("Update line meshes");
			for (auto& entity : ecs.getView<DebugBoundingBoxComponent>()) {
				if (!ecs.has<LineMeshComponent>(entity)) {
					auto box = ecs.getComponent<BoundingBoxComponent>(entity);
					auto line = ecs.addComponent<LineMeshComponent>(entity, resourceManagers.mMeshManager);

					line->mUseParentSpatial = true;
					line->mOverrideColor = box->mStatic ? glm::vec3(1.f, 0.f, 0.f) : util::genRandomVec3(0.3f, 1.f);

					glm::vec3 NearLeftBottom{ box->mMin };
					glm::vec3 NearLeftTop{ box->mMin.x, box->mMax.y, box->mMin.z };
					glm::vec3 NearRightBottom{ box->mMax.x, box->mMin.y, box->mMin.z };
					glm::vec3 NearRightTop{ box->mMax.x, box->mMax.y, box->mMin.z };
					glm::vec3 FarLeftBottom{ box->mMin.x, box->mMin.y,  box->mMax.z };
					glm::vec3 FarLeftTop{ box->mMin.x, box->mMax.y,	 box->mMax.z };
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
	}

	void Engine::_endFrame(util::Profiler& profiler, ECS& ecs) {
		TRACY_ZONE();

		for(auto& entity : ecs.getView<SingleFrameComponent>()) {
			ecs.removeEntity(entity);
		}

		for (auto&& [removeEntity, removeJob] : ecs.getView<RemoveAsyncJobComponent>().each()) {
			bool jobFound = false;
			for (auto&& [jobEntity, job] : ecs.getView<AsyncJobComponent>().each()) {
				if (removeJob.mPid == job.mPid) {
					ecs.removeEntity(removeEntity);
					ecs.removeEntity(jobEntity);
					jobFound = true;
					break;
				}
			}
			NEO_ASSERT(jobFound, "Trying to remove a non-existant async job?");
		}

		// Update display, mouse, keyboard 
		// Do it here so it coincides w/ vsync instead of stalling engine tick
		mWindow.updateHardware();

		profiler.markFrame(glfwGetTime());
	}
}
