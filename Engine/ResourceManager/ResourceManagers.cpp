#include "ResourceManager/ResourceManagers.hpp"

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
}