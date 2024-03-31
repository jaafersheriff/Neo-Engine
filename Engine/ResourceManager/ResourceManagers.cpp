#include "ResourceManager/ResourceManagers.hpp"

#include <imgui.h>

namespace neo {

	void ResourceManagers::tick() {
		TRACY_ZONE();
		mMeshManager.tick();
		mShaderManager.tick();
		mTextureManager.tick();
		mFramebufferManager.tick(mTextureManager); // Do this after textures
	}

	void ResourceManagers::clear() {
		mMeshManager.clear();
		mShaderManager.clear();
		mTextureManager.clear();
		mFramebufferManager.clear(mTextureManager); // Do this after textures
	}

	void ResourceManagers::imguiEditor() {
		ImGui::Begin("Resources");
		mFramebufferManager.imguiEditor();
		mShaderManager.imguiEditor();
		mMeshManager.imguiEditor();
		mTextureManager.imguiEditor();
		ImGui::End();
	}
}