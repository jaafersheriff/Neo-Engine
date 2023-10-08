#include "Base/BaseDemo.hpp"
#include "Compute/Compute.hpp"
#include "FrustaFitting/FrustaFitting.hpp"
<<<<<<< HEAD
#include "Cornell/Cornell.hpp"
// #include "Deferred/Deferred.hpp"
// #include "Metaballs/Metaballs.hpp"
=======
#include "Deferred/Deferred.hpp"
>>>>>>> fe811494 (Deferred?)
#include "NormalVisualizer/NormalVisualizer.hpp"
#include "PostProcess/PostProcess.hpp"
#include "Sponza/Sponza.hpp"

#include <vector>
#include <memory>

static int sCurrentDemo = 0;
static std::vector<neo::IDemo*> sDemos = {
	new Base::Demo(),
<<<<<<< HEAD
	new Cornell::Demo(),
=======
	new Sponza::Demo(),
>>>>>>> fe811494 (Deferred?)
 	new FrustaFitting::Demo(),
 	new Compute::Demo(),
	new Deferred::Demo(),
	new NormalVisualizer::Demo(),
	new PostProcess::Demo(),
};

