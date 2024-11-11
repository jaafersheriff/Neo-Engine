#pragma once

#include "Renderer/FrameGraph/FrameGraph.hpp"

namespace neo {

	void GLFrameGraphResolve(const FrameData& frameData, const Pass& pass, const Command& command, const ResourceManagers& resourceManagers);
}