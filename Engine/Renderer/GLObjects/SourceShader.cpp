#include "Renderer/pch.hpp"

#include "SourceShader.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Loader/Library.hpp"

#include <imgui.h>

namespace neo {
	SourceShader::SourceShader(const char* name, const ConstructionArgs& args) 
		: mName(name)
		, mConstructionArgs(args) 
	{
		for (auto& arg : args) {
			mShaderSources.emplace(arg.first, Loader::loadFileString(arg.second));
		}
	}

	// Don't put this in a hot loop
	SourceShader::SourceShader(const char* name, const ShaderCode& sources)
		: mName(name)
		, mShaderSources(sources) {
	}

	SourceShader::~SourceShader() {
		destroy();
	}

	void SourceShader::destroy() {
		for (auto& instance : mResolvedShaders) {
			instance.second.destroy();
		}
		mResolvedShaders.clear();

		if (mConstructionArgs) {
			for (auto& source : mShaderSources) {
				delete source.second;
			}
		}
		mShaderSources.clear();
	}

	const ResolvedShaderInstance& SourceShader::getResolvedInstance(const ShaderDefines& defines) {
		TRACY_ZONE();
		HashedShaderDefines hash = _getDefinesHash(defines);
		auto it = mResolvedShaders.find(hash);
		if (it == mResolvedShaders.end()) {
			mResolvedShaders.emplace(hash, ResolvedShaderInstance());
			it = mResolvedShaders.find(hash);
			it->second.init(mShaderSources, defines);

			std::stringstream ss;
			ss << "with {";
			for (auto d = defines.begin(); d != defines.end(); d++) {
				ss << d->c_str();
				if (d != std::prev(defines.end())) {
					ss << ", ";
				}

			}
			ss << "}";
			if (it->second.mValid) {
				NEO_LOG_I("Resolving a new shader for %s %s", mName.c_str(), ss.str().c_str());
			}
			else {
				NEO_LOG_E("Failed to resolve instance of %s %s", mName.c_str(), ss.str().c_str());
			}
		}

		if (it->second.mValid) {
			return it->second;
		}

		return Library::getDummyShader();
	}

	SourceShader::HashedShaderDefines SourceShader::_getDefinesHash(const SourceShader::ShaderDefines& defines) {
		HashedShaderDefines seed = static_cast<HashedShaderDefines>(defines.size());
		for (auto& i : defines) {
			seed ^= HashedString(i.c_str()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}

	void SourceShader::imguiEditor() {
		ImGui::Text("Variants: %d", mResolvedShaders.size());
		if (mConstructionArgs && ImGui::Button("Reload")) {
			destroy();
			for (auto& arg : *mConstructionArgs) {
				mShaderSources.emplace(arg.first, Loader::loadFileString(arg.second));
			}
		}
	}
}
