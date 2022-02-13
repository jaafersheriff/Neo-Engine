#include "Engine/Engine.hpp"
#include "DemoInfra/IDemo.hpp"
#include "DemoRegistration.h"

#include <cstdlib>
#include <vector>
#include <memory>

int main() {
    using namespace neo;

    Engine::init();
    Engine::run(sDemos, sCurrentDemo);

	return EXIT_SUCCESS;
}
