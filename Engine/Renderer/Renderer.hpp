#pragma once

#include "ResourceManager/TextureManager.hpp"
#include "DemoInfra/IDemo.hpp"

#include "HAL/FrameStats.hpp"
#include "HAL/GpuTimer.hpp"
#include "HAL/RenderDetails.hpp"

#include "Util/Profiler.hpp"

#include <typeindex>
#include <memory>
#include <tuple>

namespace neo {

	class Engine;
	class ImGuiManager;
	class ECS;
	class WindowSurface;
	class PostProcessShader;
	class SourceShader;
	struct FrameSizeMessage;
	class ResourceManagers;

	class Renderer {

		//friend ImGuiManager;
		friend Engine;

		public:
			Renderer(int GLMajor, int GLMinor);
			~Renderer();
			Renderer(const Renderer &) = delete;
			Renderer & operator=(const Renderer &) = delete;
			Renderer(Renderer &&) = delete;
			Renderer & operator=(Renderer &&) = delete;

			RendererDetails getDetails() const { return mDetails; }

			void setDemoConfig(IDemo::Config);

			// Binds the GPU context to the calling thread
			void initGPUContext(WindowSurface& window);

			// Sets initial state
			void init();
			void render(WindowSurface&, IDemo* demo, util::Profiler& profiler, const ECS&, ResourceManagers& resourceManager);
			void clean();

		private:
			void _imGuiEditor(WindowSurface& window, ECS& ecs, ResourceManagers& resourceManager);

			// Copied from GLDevice::stats() at the end of a frame, for reads on the main thread
			FrameStats mPreviousStats = {};
			RendererDetails mDetails = {};

			// Assigned on the render thread, read by ImGui on main without sync
			// Should be safe - the worst outcome is that isValid() fails and systems gracefully return early
			TextureHandle mSceneColorTextureHandle;

			GpuTimer mGPUQuery;
	};

}
