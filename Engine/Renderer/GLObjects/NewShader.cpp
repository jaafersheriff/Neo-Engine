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
		for (auto& source : mConstructionArgs) {
			delete source.second;
		}
		mConstructionArgs.clear();
	}

	void NewShader::destroy() {
		for (auto& instance : mResolvedShaders) {
			instance.second.destroy();
		}
		mResolvedShaders.clear();
	}

	const ResolvedShaderInstance& NewShader::getResolvedInstance(const ShaderDefines& defines) {
        MICROPROFILE_SCOPEI("ResolvedShaderInstance", "getResolvedInstance", MP_AUTO);
		HashedShaderDefines hash = _getDefinesHash(defines);
		auto it = mResolvedShaders.find(hash);
		if (it == mResolvedShaders.end()) {
			mResolvedShaders.emplace(hash, ResolvedShaderInstance());
			it = mResolvedShaders.find(hash);
			it->second.init(mConstructionArgs, defines);

			std::stringstream ss;
#ifdef DEBUG_MODE
			ss << "{";
			for (auto& s : defines) {
				ss << s.c_str() << ", ";
			}
			ss << "}";
#endif
			if (it->second.mValid) {
				NEO_LOG_V("Resolving a new shader for %s with %s", mName.c_str(), ss.str().c_str());
			}
			else {
				NEO_LOG_E("Failed to resolve instance of %s with %s", mName.c_str(), ss.str().c_str());
			}
		}


		if (it->second.mValid) {
			return it->second;
		}

		return Library::getDummyShader();
	}

	NewShader::HashedShaderDefines NewShader::_getDefinesHash(const NewShader::ShaderDefines& defines) {
		HashedShaderDefines seed = static_cast<HashedShaderDefines>(defines.size());
		for (auto& i : defines) {
			seed ^= HashedString(i.c_str()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
}