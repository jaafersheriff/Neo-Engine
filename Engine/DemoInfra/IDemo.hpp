#pragma once

#include "Util/Util.hpp"
#include <string>

namespace neo {

	class ECS;
	class Renderer;
	class Framebuffer;
	class ResourceManagers;

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
		virtual void render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) = 0;
		virtual void imGuiEditor(ECS& ecs) {
			NEO_UNUSED(ecs);
		}
		virtual void destroy() {}

	private:
	};
}
