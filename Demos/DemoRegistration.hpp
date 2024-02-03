#include "Base/BaseDemo.hpp"
#include "Sponza/Sponza.hpp"
#include "Compute/Compute.hpp"
#include "Cornell/Cornell.hpp"
#include "DrawStress/DrawStress.hpp"
// #include "FrustaFitting/FrustaFitting.hpp"
#include "NormalVisualizer/NormalVisualizer.hpp"

#include <vector>
#include <memory>

static int sCurrentDemo = 0;
static std::vector<neo::IDemo*> sDemos = {
	new Sponza::Demo(),
	new Base::Demo(),
 	new Compute::Demo(),
	new Cornell::Demo(),
	new DrawStress::Demo(),
 	// new FrustaFitting::Demo(),
	new NormalVisualizer::Demo(),
};

