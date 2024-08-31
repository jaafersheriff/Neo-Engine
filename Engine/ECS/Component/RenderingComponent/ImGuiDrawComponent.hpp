#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/MeshManager.hpp"

namespace neo {
	START_COMPONENT(ImGuiDrawComponent);
		ImGuiDrawComponent() {}
		MeshHandle mMeshHandle = NEO_INVALID_HANDLE;
		TextureHandle mTextureHandle = NEO_INVALID_HANDLE;
		glm::uvec4 mScissorRect;
		uint16_t mElementCount = 0;
		uint16_t* mElementBufferOffset = 0;

	END_COMPONENT();
}