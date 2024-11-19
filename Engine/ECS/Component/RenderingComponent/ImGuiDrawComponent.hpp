#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/MeshManager.hpp"
#include "ResourceManager/TextureManager.hpp"

namespace neo {
	// Needed for views
	START_COMPONENT(ImGuiComponent);
	END_COMPONENT();

	START_COMPONENT(ImGuiDrawComponent);
		ImGuiDrawComponent() {}
		MeshHandle mMeshHandle = NEO_INVALID_HANDLE;
		TextureHandle mTextureHandle = NEO_INVALID_HANDLE;
		glm::uvec4 mScissorRect;
		uint16_t mElementCount = 0;
		uint16_t mElementBufferOffset = 0;
		uint32_t mDrawOrder = 0;
	END_COMPONENT();

	START_COMPONENT(ImGuiMeshViewComponent);
		ImGuiMeshViewComponent() {}
		MeshHandle mMeshHandle = NEO_INVALID_HANDLE;
		glm::uvec4 mBounds;
		glm::vec3 mMin, mMax;
	END_COMPONENT();



}