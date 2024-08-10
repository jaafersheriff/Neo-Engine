#include "Base/BaseDemo.hpp"
#include "Compute/Compute.hpp"
#include "Cornell/Cornell.hpp"
#include "DrawStress/DrawStress.hpp"
#include "Fireworks/FireworkDemo.hpp"
#include "FrustaFitting/FrustaFitting.hpp"
#include "NormalVisualizer/NormalVisualizer.hpp"
#include "DeferredPBR/DeferredPBR.hpp"

#include <vector>
#include <memory>

static int sCurrentDemo = 0;
static std::vector<neo::IDemo*> sDemos = {
	new Base::Demo(),
	new DeferredPBR::Demo(),
	new Compute::Demo(),
	new Fireworks::Demo(),
	new Cornell::Demo(),
	new DrawStress::Demo(),
	new FrustaFitting::Demo(),
	new NormalVisualizer::Demo(),
};

