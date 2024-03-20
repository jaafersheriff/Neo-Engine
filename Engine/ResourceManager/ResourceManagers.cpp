#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	void ResourceManagers::tick() {
		TRACY_ZONE();
		mMeshManager._tick();
		mShaderManager._tick();
	}

	void ResourceManagers::clear() {
		mMeshManager.clear();
		mShaderManager.clear();
	}
}