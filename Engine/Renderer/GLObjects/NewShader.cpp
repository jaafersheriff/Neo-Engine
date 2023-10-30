#include "Renderer/pch.hpp"

#include "NewShader.hpp"

#include "GLHelper.hpp"

namespace neo {
	NewShader::NewShader(const ConstructionArgs& args) 
		: mConstructionArgs(args) {
	}

	NewShader::~NewShader() {
		mResolvedShaders.clear();// TODO - more to be done here
	}

	ResolvedShaderInstance NewShader::getResolvedInstance(const ResolvedShaderInstance::ShaderDefines& defines) {
		auto it = mResolvedShaders.find(defines);
		if (it != mResolvedShaders.end()) {
			return it->second;
		}
		else {
			mResolvedShaders.emplace(ResolvedShaderInstance{
				// TODO - need to make a new shader..
			});
		}
	}
}