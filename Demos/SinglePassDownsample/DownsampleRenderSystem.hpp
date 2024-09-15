#pragma once

#include "ResourceManager/ResourceManagers.hpp"

namespace SPD {
	neo::TextureHandle downSample(neo::TextureHandle inputDepthHandle, const neo::ResourceManagers& resourceManagers);

	struct DownSampleBlitParameters{
		uint8_t mip = 0;

		void imguiEditor();
	};
	void downSampleDebugBlit(neo::Framebuffer& outputFBO, neo::TextureHandle hiz, const neo::ResourceManagers& resourceManagers, DownSampleBlitParameters params);
}