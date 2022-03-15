#include "Engine/Engine.hpp"
#include "DemoInfra/DemoWrangler.h"
#include "DemoRegistration.hpp"

#include <cstdlib>
#include <vector>
#include <memory>

int main() {
    neo::DemoWrangler demos(sCurrentDemo, sDemos);
    neo::Engine::init();
    neo::Engine::run(demos);

	return EXIT_SUCCESS;
}
