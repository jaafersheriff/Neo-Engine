#pragma once

#include "ResourceManager/FramebufferManager.hpp"

#include "Util/Util.hpp"
#include <string>

namespace neo {

	class ECS;
	class Renderer;
	class ResourceManagers;
	class FrameGraph;

	class IDemo {
	public:
		struct Config {
			std::string name = "";
			std::string resDir = "res/";
			std::string shaderDir = "shaders/";
		};

		/* Construction flow goes through init/destroy */
		IDemo() = default;
		~IDemo() = default;
		IDemo(const IDemo&) = delete;
		IDemo& operator=(const IDemo&) = delete;

		virtual Config getConfig() const = 0;
		virtual void init(ECS& ecs, ResourceManagers& resourceManagers) = 0;
		virtual void update(ECS& ecs, ResourceManagers& resourceManagers) {
			NEO_UNUSED(ecs, resourceManagers);
		};
		virtual void render(FrameGraph& fg, const ResourceManagers& resourceManagers, const ECS& ecs, FramebufferHandle backbufferHandle) = 0;
		virtual void imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) {
			NEO_UNUSED(ecs, resourceManagers);
		}
		virtual void destroy() {}

	private:
	};
}
