#pragma once

#include "DemoInfra/IDemo.hpp"

#include "ResourceManager/FramebufferManager.hpp"

#include <typeindex>
#include <memory>
#include <tuple>

namespace neo {

	class Engine;
	class ImGuiManager;
	class ECS;
	class WindowSurface;
	class PostProcessShader;
	class Framebuffer;
	class SourceShader;
	struct FrameSizeMessage;
	class ResourceManagers;

	class Renderer {

		friend ImGuiManager;

		// This should be moved to its own file/service locator..?
		struct FrameStats {
			uint32_t mNumDraws = 0;
			uint32_t mNumPrimitives = 0;
			uint32_t mNumUniforms = 0;
			uint32_t mNumSamplers = 0;
		};

		struct RendererDetails {
			int mGLMajorVersion = 0;
			int mGLMinorVersion = 0;
			std::string mGLSLVersion = "";
			glm::ivec3 mMaxComputeWorkGroupSize = { 0,0,0 };
			std::string mVendor = "";
			std::string mRenderer = "";
			std::string mShadingLanguage = "";
		};

		public:
			Renderer(int GLMajor, int GLMinor);
			~Renderer();
			Renderer(const Renderer &) = delete;
			Renderer & operator=(const Renderer &) = delete;
			Renderer(Renderer &&) = delete;
			Renderer & operator=(Renderer &&) = delete;

			FrameStats mStats = {};
			RendererDetails mDetails = {};

			void setDemoConfig(IDemo::Config);
			void init();
			void resetState();
			void render(WindowSurface&, IDemo* demo, ECS&, ResourceManagers& resourceManager);
			void clean();

			void imGuiEditor(WindowSurface& window, ECS& ecs, ResourceManagers& resourceManager);
		private:
			FramebufferHandle mDefaultFBOHandle;
			bool mShowBoundingBoxes = false;
			bool mBackbufferNeedsResize = false;
	};

}
