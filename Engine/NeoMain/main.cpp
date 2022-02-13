#include "Engine/Engine.hpp"
#include "DemoInfra/IDemo.hpp"
#include "DemoRegistration.h"

#include <cstdlib>
#include <vector>
#include <memory>

int main() {
    using namespace neo;

    DemoWrangler demos = {
        sCurrentDemo,
        sDemos
    };
    Engine::init();
    Engine::run(demos);

	return EXIT_SUCCESS;
}
