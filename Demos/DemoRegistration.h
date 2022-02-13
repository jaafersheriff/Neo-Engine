#include "Base/BaseDemo.hpp"
#include "BasicPhong/BasicPhong.hpp"
#include <vector>
#include <memory>

static uint32_t sCurrentDemo = 0;
static std::vector<neo::IDemo*> sDemos = {
	// TODO - maybe there should be some internal demo
	new BaseDemo(),
	new BasicPhong()
};

