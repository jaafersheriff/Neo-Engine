#include "Engine/Engine.hpp"
#include "DemoInfra/DemoWrangler.hpp"
#include "DemoRegistration.hpp"

#include <cstdlib>
#include <vector>
#include <memory>

int main() {
	neo::Engine engine;
	engine.init();
	engine.run(std::move(neo::DemoWrangler(sCurrentDemo, sDemos)));

	return EXIT_SUCCESS;
}
