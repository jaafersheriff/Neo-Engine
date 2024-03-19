#pragma once

#include "Util/Util.hpp"
#include <string>

namespace neo {

	class ECS;
	class Renderer;
	class Framebuffer;
	class MeshManager;

	class IDemo {
	public:
		struct Config {
			std::string name = "";
			std::string resDir = "res/";
			std::string shaderDir = "shaders/";
			glm::vec3 clearColor = { 0.2f, 0.3f, 0.4f };
		};

		/* Construction flow goes through init/destroy */
		IDemo() = default;
		~IDemo() = default;
		IDemo(const IDemo&) = delete;
		IDemo& operator=(const IDemo&) = delete;

		virtual Config getConfig() const = 0;
		// TODO - meshmanager could just be added to the ecs probably..but how would that work with threading
		virtual void init(ECS& ecs, MeshManager& meshManager) = 0;
		virtual void update(ECS& ecs) {
			NEO_UNUSED(ecs);
		};
		virtual void render(const MeshManager& meshManager, const ECS& ecs, Framebuffer& backbuffer) = 0;
		virtual void imGuiEditor(ECS& ecs) {
			NEO_UNUSED(ecs);
		}
		virtual void destroy() {}

	private:
	};
}
