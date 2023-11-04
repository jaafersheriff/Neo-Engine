#include "Renderer/pch.hpp"

#include "NewShader.hpp"

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Loader/Library.hpp"

namespace neo {
	NewShader::NewShader(const char* name, const ConstructionArgs& args) 
		: mConstructionArgs(args) 
		, mName(name)
	{
	}

	NewShader::~NewShader() {
		// TODO - delete the construction args file strings
		mResolvedShaders.clear();
	}

	const ResolvedShaderInstance& NewShader::getResolvedInstance(const ShaderDefines& defines) {
		HashedShaderDefines hash = _getDefinesHash(defines);
		auto it = mResolvedShaders.find(hash);
		if (it == mResolvedShaders.end()) {
			mResolvedShaders.emplace(std::make_pair(hash, ResolvedShaderInstance(
				mConstructionArgs,
				defines
			)));
			it = mResolvedShaders.find(hash);
		}
		if (it->second.mValid) {
			return it->second;
		}

		NEO_LOG_E("Failed to find or compile %s", mName.c_str());

		// TODO - return dummy shader..
		return Library::getDummyShader();
	}
}