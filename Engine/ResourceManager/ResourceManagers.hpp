#pragma once

#include "MeshResourceManager.hpp"
#include "ShaderResourceManager.hpp"

#include "Util/Profiler.hpp"

namespace neo {

	class Engine;

	class ResourceManagers {
		friend Engine;
	public:
		MeshResourceManager mMeshManager;
		ShaderResourceManager mShaderManager;
	private:
		void clear();
		void tick();
	};
}