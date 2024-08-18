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
			void shutDown(ECS& ecs, ResourceManagers& resourceManagers);

		private:
			void _startFrame(util::Profiler& profiler, ECS& ecs, ResourceManagers& resourceManagers);
			void _endFrame(ECS& ecs);

			void _createPrefabs(ResourceManagers& resourceManagers);
			void _swapDemo(DemoWrangler& demoWranger, ECS& ecs, ResourceManagers& resourceManagers);

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