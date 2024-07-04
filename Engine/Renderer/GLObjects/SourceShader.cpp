#include "Renderer/pch.hpp"

#include "SourceShader.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

namespace neo {

	// Don't put this in a hot loop
	SourceShader::SourceShader(const char* name, const ShaderCode& sources)
		: mName(name)
		, mShaderSources(sources) {
	}

	void SourceShader::destroy() {
		for (auto& instance : mResolvedShaders) {
			instance.second.destroy();
		}
		mResolvedShaders.clear();
		mShaderSources.clear();
	}

	const ResolvedShaderInstance& SourceShader::getResolvedInstance(const ShaderDefines& defines) const {
		HashedShaderDefines hash = _getDefinesHash(defines);
		auto it = mResolvedShaders.find(hash);
		if (it == mResolvedShaders.end()) {
			mResolvedShaders.emplace(hash, ResolvedShaderInstance(defines));
			it = mResolvedShaders.find(hash);
			it->second.init(mShaderSources, defines);

			std::stringstream ss;
			if (defines.mDefines.size() || defines.mParent != nullptr) {
				ss << "with\n";
				const ShaderDefines* _defines = &defines;
				while (_defines) {
					for (auto& define : _defines->mDefines) {
						if (define.second) {
							ss << "\t" << define.first.mVal.data() << "\n";
						}
					}

					_defines = _defines->mParent;
				}
			}
			// Remove the last newline hehe
			ss.seekp(-1, ss.cur); ss << '\0';
			if (it->second.isValid()) {
				NEO_LOG_I("Resolving a new variant for %s %s", mName.c_str(), ss.str().c_str());
			}
			else {
				NEO_LOG_E("Failed to resolve instance of %s %s", mName.c_str(), ss.str().c_str());
			}
		}

		return it->second;
	}

	SourceShader::HashedShaderDefines SourceShader::_getDefinesHash(const ShaderDefines& defines) const {
		HashedShaderDefines seed = static_cast<HashedShaderDefines>(defines.mDefines.size());
		const ShaderDefines* _defines = &defines;
		while (_defines) {
			for (auto& define : _defines->mDefines) {
				if (define.second) {
					seed ^= define.first.mVal.value() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				}
			}

			_defines = _defines->mParent;
		}

		return seed;
	}
}
