#include "Engine/Engine.hpp"
#include "DemoInfra/IDemo.hpp"
#include "DemoRegistration.h"

#include <cstdlib>
#include <vector>
#include <memory>

int main() {
    using namespace neo;

    EngineConfig config;
    config.APP_NAME = "";
    config.APP_RES = "res/";
    Engine::init(config);

    Engine::run(sDemos, sCurrentDemo);

	return EXIT_SUCCESS;
}
