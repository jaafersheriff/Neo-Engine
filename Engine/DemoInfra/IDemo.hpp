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
			glm::vec3 clearColor = { 0.f, 0.f, 0.f };
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
		virtual void destroy() {}

	private:
	};

	class DemoWrangler {
	public:
		int& currentDemoIndex;
		std::vector<IDemo*>& demos;

		void SetDemo(int newDemo) { currentDemoIndex = newDemo; }
		IDemo::Config GetConfig() { return demos[currentDemoIndex]->getConfig(); }
		IDemo* CurrentDemo() { return demos[currentDemoIndex]; }

	};
}
