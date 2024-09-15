#pragma once

#include "ResourceManager/ResourceManagers.hpp"

namespace SPD {
	neo::TextureHandle downSample(neo::TextureHandle inputDepthHandle, const neo::ResourceManagers& resourceManagers);
	void downSampleDebugBlit(neo::Framebuffer& outputFBO, neo::TextureHandle hiz, const neo::ResourceManagers& resourceManagers);
}