#pragma once

#include "MeshResourceManager.hpp"
#include "ShaderResourceManager.hpp"
#include "TextureResourceManager.hpp"
#include "FramebufferManager.hpp"

#include "Util/Profiler.hpp"

namespace neo {

	class Engine;


	class ResourceManagers {
		friend Engine;
	public:
		MeshResourceManager mMeshManager;
		ShaderResourceManager mShaderManager;
		TextureResourceManager mTextureManager;
		FramebufferResourceManager mFramebufferManager;
	private:
		void imguiEditor();
		void clear();
		void tick();
	};
}