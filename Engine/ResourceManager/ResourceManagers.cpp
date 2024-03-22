#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	void ResourceManagers::tick() {
		TRACY_ZONE();
		mMeshManager.tick();
		mShaderManager._tick();
		mTextureManager.tick();
	}

	void ResourceManagers::clear() {
		mMeshManager.clear();
		mShaderManager.clear();
		mTextureManager.clear();
	}
}