#include "Engine/Engine.hpp"
#include "DemoInfra/DemoWrangler.hpp"
#include "DemoRegistration.hpp"

// CP1a scaffolding. Proves the include path, the ENKI_ASSERT wrapper header and the link all work
// before any Neo code depends on enkiTS. Both includes and the log below come back out in CP1b,
// when Engine/Jobs becomes the only thing that touches enkiTS.
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
