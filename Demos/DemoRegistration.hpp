#include "Base/BaseDemo.hpp"
#include "Compute/Compute.hpp"
#include "FrustaFitting/FrustaFitting.hpp"
#include "Cornell/Cornell.hpp"
#include "NormalVisualizer/NormalVisualizer.hpp"
#include "Sponza/Sponza.hpp"

#include <vector>
#include <memory>

static int sCurrentDemo = 0;
static std::vector<neo::IDemo*> sDemos = {
	new Base::Demo(),
	new Cornell::Demo(),
	new Sponza::Demo(),
 	new FrustaFitting::Demo(),
 	new Compute::Demo(),
	new NormalVisualizer::Demo(),
};

