#pragma once

#include "ResourceManager/TextureManager.hpp"
#include "DemoInfra/IDemo.hpp"

#include "FrameStats.hpp"
#include "RenderDetails.hpp"

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

			FrameStats mStats = {};

			RendererDetails getDetails() const { return mDetails; }

			void setDemoConfig(IDemo::Config);
			void init();
			void render(WindowSurface&, IDemo* demo, util::Profiler& profiler, const ECS&, ResourceManagers& resourceManager);
			void clean();

		private:
			void _imGuiEditor(WindowSurface& window, ECS& ecs, ResourceManagers& resourceManager);

			RendererDetails mDetails = {};

			TextureHandle mSceneColorTextureHandle;
			bool mShowBoundingBoxes = false;
			bool mWireframe = false;

			util::Profiler::GPUQuery mGPUQuery;
	};

}
