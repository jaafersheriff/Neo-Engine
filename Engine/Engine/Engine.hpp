#pragma once

#include "Renderer/Renderer.hpp"
#include "Util/Util.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Systems/CollisionSystems/MouseRaySystem.hpp"
#include "ECS/Systems/CollisionSystems/SelectingSystem.hpp"

#include "DemoInfra/DemoWrangler.hpp"

#include "Hardware/WindowSurface.hpp"
#include "Hardware/Keyboard.hpp"
#include "Hardware/Mouse.hpp"

namespace neo {
	namespace util {
		struct FrameCounter;
	}
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

			void init();
			void run(DemoWrangler&& demoWrangler);
			void shutDown(ResourceManagers& resourceManagers);

		private:
			ECS mECS;
			// TODO - managers could just be added to the ecs probably..but how would that work with threading
			void _startFrame(util::Profiler& profiler, ResourceManagers& resourceManagers);
			void _endFrame();

			void _createPrefabs(ResourceManagers& resourceManagers);
			void _swapDemo(DemoWrangler& demoWranger, ResourceManagers& resourceManagers);

			/* Hardware */
			WindowSurface mWindow;
			Keyboard mKeyboard;
			Mouse mMouse;

			/* Debug */
			bool mShowBoundingBoxes = false;
			MouseRaySystem mMouseRaySystem;
			SelectingSystem mSelectingSystem;
	};

}