#include "Engine/Engine.hpp"
#include "DemoInfra/DemoWrangler.hpp"
#include "DemoRegistration.hpp"

// TODO - Temp scaffolding, remove
#include <ext/enki_incl.hpp>
#include "Util/Log/Log.hpp"

#include <cstdlib>
#include <vector>
#include <memory>

int main() {
	NEO_LOG_I("enkiTS sees %u hardware threads", enki::GetNumHardwareThreads());

	neo::Engine engine;
	engine.init();
	engine.run(std::move(neo::DemoWrangler(sCurrentDemo, sDemos)));

	return EXIT_SUCCESS;
}
