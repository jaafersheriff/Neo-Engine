#include "Base/BaseDemo.hpp"
#include "Compute/Compute.hpp"
#include "FrustaFitting/FrustaFitting.hpp"
// #include "Deferred/Deferred.hpp"
// #include "GodRays/GodRays.hpp"
// #include "Metaballs/Metaballs.hpp"
// #include "NormalVisualizer/NormalVisualizer.hpp"
// #include "PostProcess/PostProcess.hpp"
// #include "Selecting/Selecting.hpp"
#include "Sponza/Sponza.hpp"

#include <vector>
#include <memory>

static int sCurrentDemo = 0;
static std::vector<neo::IDemo*> sDemos = {
	new Base::Demo(),
 	new FrustaFitting::Demo(),
 	new Compute::Demo(),
// 	new Deferred::Demo(),
// 	new GodRays::Demo(),
// 	new Metaballs::Demo(),
// 	new NormalVisualizer::Demo(),
// 	new PostProcess::Demo(),
// 	new Selecting::Demo(),
	new Sponza::Demo(),
};
