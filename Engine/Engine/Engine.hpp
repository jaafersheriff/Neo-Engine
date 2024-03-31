#pragma once

#include "Renderer/Renderer.hpp"
#include "Util/Util.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Systems/CollisionSystems/MouseRaySystem.hpp"
#include "ECS/Systems/CollisionSystems/SelectingSystem.hpp"

#include "DemoInfra/DemoWrangler.hpp"

namespace neo {
	namespace util {
		struct FrameCounter;
	}
	class Window;
	class Keyboard;
	class Mouse;
	class ResourceManagers;

	class Engine {

		/* Base Engine */
		public:
			Engine() = default;
			~Engine() = default;
			Engine(const Engine &) = delete;
			Engine & operator=(const Engine &) = delete;
			Engine(Engine &&) = delete;
			Engine & operator=(Engine &&) = delete;

			static void init();
			static void run(DemoWrangler& demoWrangler);
			static void shutDown(ResourceManagers& resourceManagers);

		private:
			static ECS mECS;
			// TODO - managers could just be added to the ecs probably..but how would that work with threading
			static void _startFrame(util::Profiler& profiler, ResourceManagers& resourceManagers);
			static void _endFrame();

			static void _createPrefabs(ResourceManagers& resourceManagers);
			static void _swapDemo(DemoWrangler& demoWranger, ResourceManagers& resourceManagers);

			/* Hardware */
			static WindowSurface mWindow;
			static Keyboard mKeyboard;
			static Mouse mMouse;

			/* Debug */
			bool mShowBoundingBoxes = false;
			static MouseRaySystem mMouseRaySystem;
			static SelectingSystem mSelectingSystem;
	};

}