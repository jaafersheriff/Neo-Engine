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

	const ResolvedShaderInstance& SourceShader::getResolvedInstance(const std::vector<ShaderDefinesFG>& defines) const {
		const ShaderDefinesFG::HashedShaderDefines hash = ShaderDefinesFG::getDefinesHash(defines);
		auto it = mResolvedShaders.find(hash);
		if (it == mResolvedShaders.end()) {

			std::stringstream ss;
			ss << "\n";
			for (const auto& define : defines) {
				for (uint8_t i = 0; i < define.getDefinesSize(); i++) {
					ss << "\t" << define.getDefine(i) << "\n";
				}
			}

			mResolvedShaders.emplace(hash, ResolvedShaderInstance(ss.str()));
			it = mResolvedShaders.find(hash);
			it->second.init(mShaderSources, defines);

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
}
