#include "Base/BaseDemo.hpp"
#include "BasicPhong/BasicPhong.hpp"
#include "Compute/Compute.hpp"
#include "VFC/VFC.hpp"

#include <vector>
#include <memory>

static int sCurrentDemo = 0;
static std::vector<neo::IDemo*> sDemos = {
	new BaseDemo(),
	new BasicPhong(),
	new Compute(),
	new VFC()
};

