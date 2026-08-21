#pragma once

#include "Renderer/Renderer.hpp"
#include "Jobs/JobSystem.hpp"
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
			void _endFrame(util::Profiler& profiler, ECS& ecs);

			void _createPrefabs(ResourceManagers& resourceManagers);
			void _swapDemo(DemoWrangler& demoWranger, ECS& ecs, ResourceManagers& resourceManagers);

			void _imguiEditor(ECS& ecs);

			/* Hardware */
			WindowSurface mWindow;
			Keyboard mKeyboard;
			Mouse mMouse;

			/* Render's view of the world: a deep copy taken once per frame, so the renderer never
			   reads a registry the simulation is still mutating. Double-buffered because Stage 4 hands
			   one to the render thread while the main thread refills the other. */
			ECS mRenderECS[2];
			uint8_t mRenderECSIndex = 0;

			/* The frame in flight on the render job thread. Waiting on it is the handshake that keeps
			   the main thread exactly one frame ahead; assigning a new one waits on what it replaces,
			   so the double buffer can never be overwritten from under the renderer. Released in
			   shutDown() while the JobSystem is still alive, because ~JobHandle waits on it. */
			JobHandle mRenderJob;

			/* Debug */
			bool mShowBoundingBoxes = false;
			MouseRaySystem mMouseRaySystem;
			SelectingSystem mSelectingSystem;
	};

}