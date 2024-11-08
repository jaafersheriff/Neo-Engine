#pragma once

#include "MeshManager.hpp"
#include "ShaderManager.hpp"
#include "TextureManager.hpp"
#include "FramebufferManager.hpp"

#include "Util/Profiler.hpp"

namespace neo {

	class Engine;
	class Renderer;

	class ResourceManagers {
		friend Engine;
		friend Renderer;
	public:
		MeshManager mMeshManager;
		ShaderManager mShaderManager;
		TextureManager mTextureManager;
		FramebufferManager mFramebufferManager;
	private:
		void _imguiEditor();
		void _clear();
		void _tick();
	};
}