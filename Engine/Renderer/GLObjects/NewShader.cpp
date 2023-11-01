#include "Renderer/pch.hpp"

#include "NewShader.hpp"

#include "Loader/Library.hpp"
#include "GLHelper.hpp"

namespace neo {
	NewShader::NewShader(const char* name, const ConstructionArgs& args) 
		: mConstructionArgs(args) 
		, mName(name)
	{
	}

	NewShader::~NewShader() {
		mResolvedShaders.clear();
	}

	ResolvedShaderInstance NewShader::getResolvedInstance(const ResolvedShaderInstance::ShaderDefines& defines) {
		auto it = mResolvedShaders.find(defines);
		if (it != mResolvedShaders.end()) {
			return it->second;
		}
		else {
			auto newInstance = ResolvedShaderInstance(
				mConstructionArgs,
				defines
			);
			if (newInstance.mValid) {
				mResolvedShaders.emplace(newInstance);
				return newInstance;
			}
		}

		NEO_LOG_E("Failed to find or compile %s", mName.c_str());

		// TODO - return dummy shader..
		return Renderer::getDummyShader();
	}
}