#include "Base/BaseDemo.hpp"
#include "BasicPhong/BasicPhong.hpp"
#include "Compute/Compute.hpp"
#include "Deferred/Deferred.hpp"
#include "FrustaFitting/FrustaFitting.hpp"
#include "GodRays/GodRays.hpp"
#include "Metaballs/Metaballs.hpp"
#include "NormalVisualizer/NormalVisualizer.hpp"
#include "PostProcess/PostProcess.hpp"
#include "Selecting/Selecting.hpp"
#include "Sponza/Sponza.hpp"

#include <vector>
#include <memory>

static int sCurrentDemo = 0;
static std::vector<neo::IDemo*> sDemos = {
	new Base::Demo(),
	new BasicPhong::Demo(),
	new Compute::Demo(),
	new Deferred::Demo(),
	new FrustaFitting::Demo(),
	new GodRays::Demo(),
	new Metaballs::Demo(),
	new NormalVisualizer::Demo(),
	new PostProcess::Demo(),
	new Selecting::Demo(),
	new Sponza::Demo(),
};

