#pragma once

#include "Util/Util.hpp"
#include <string>

namespace neo {

	class ECS;

	class IDemo {
	public:
		struct Config {
			std::string name = "";
			std::string resDir = "res/";
			std::string shaderDir = "shaders/";
			glm::vec3 clearColor = { 0.2f, 0.3f, 0.4f };
			bool attachEditor = true;
		};

		/* Construction flow goes through init/destroy */
		IDemo() = default;
		~IDemo() = default;
		IDemo(const IDemo&) = delete;
		IDemo& operator=(const IDemo&) = delete;

		virtual Config getConfig() const = 0;
		virtual void init(ECS& ecs) = 0;
		virtual void update(ECS& ecs) {
			NEO_UNUSED(ecs);
		};
		virtual void imGuiEditor(ECS& ecs) {
			NEO_UNUSED(ecs);
		}
		virtual void destroy() {}

	private:
	};
}
