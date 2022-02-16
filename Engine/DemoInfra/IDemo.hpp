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
		DemoWrangler(int& idx, std::vector<IDemo*>& d)
			: currentDemoIndex(idx)
			, demos(d)
		{}

		int& currentDemoIndex;
		std::vector<IDemo*>& demos;

		IDemo::Config getConfig() { return demos[currentDemoIndex]->getConfig(); }
		IDemo* getCurrentDemo() { return demos[currentDemoIndex]; }

		void swap() { currentDemoIndex = nextDemoIndex; forceReload = false; }
		void reload(int newDemo) { nextDemoIndex = newDemo; }
		void setForceReload() { forceReload = true; }
		bool needsReload() { return forceReload || nextDemoIndex != currentDemoIndex; };
	private:
		int nextDemoIndex = 0;
		bool forceReload = false;
	};
}
