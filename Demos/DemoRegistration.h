#include "Base/BaseDemo.hpp"
#include "BasicPhong/BasicPhong.hpp"
#include "Compute/Compute.hpp"
#include "GodRays/GodRays.hpp"
#include "NormalVisualizer/NormalVisualizer.hpp"
#include "VFC/VFC.hpp"

#include <vector>
#include <memory>

static int sCurrentDemo = 0;
static std::vector<neo::IDemo*> sDemos = {
	new BaseDemo(),
	new BasicPhong(),
	new Compute(),
	new GodRays(),
	new NormalVisualizer(),
	new VFC()
};

