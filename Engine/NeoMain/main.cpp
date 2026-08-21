#include "Engine/Engine.hpp"
#include "DemoInfra/DemoWrangler.hpp"
#include "DemoRegistration.hpp"

// TODO - Temp scaffolding, remove
#include "Jobs/JobSystem.hpp"
#include "Util/Log/Log.hpp"

#include <cstdlib>
#include <vector>
#include <memory>

int main() {
	{
		neo::JobSystem jobs;
		jobs.init();
		NEO_LOG_I("JobSystem: %u threads - main=%u, render=%u, %u compute workers, this thread is %u",
			jobs.numThreads(),
			jobs.threadIndexOf(neo::JobThread::Main),
			jobs.threadIndexOf(neo::JobThread::Render),
			jobs.numWorkers(),
			jobs.threadIndex());
		jobs.shutdown();
	}

	neo::Engine engine;
	engine.init();
	engine.run(std::move(neo::DemoWrangler(sCurrentDemo, sDemos)));

	return EXIT_SUCCESS;
}
