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
			if (it->second.mValid) {
				NEO_LOG_I("Resolving a new variant for %s %s", mName.c_str(), ss.str().c_str());
			}
			else {
				NEO_LOG_E("Failed to resolve instance of %s %s", mName.c_str(), ss.str().c_str());
			}
		}

		if (it->second.mValid) {
			it->second.bind();
			return it->second;
		}
		Library::getDummyShader().bind();
		return Library::getDummyShader();
	}

	SourceShader::HashedShaderDefines SourceShader::_getDefinesHash(const ShaderDefines& defines) {
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

	void SourceShader::imguiEditor() {
		if (mConstructionArgs && ImGui::Button("Reload all")) {
			destroy();
			for (auto& arg : *mConstructionArgs) {
				mShaderSources.emplace(arg.first, Loader::loadFileString(arg.second));
			}
		}

		if (mResolvedShaders.size()) {
			if (ImGui::TreeNode("##idk", "Variants (%d)", static_cast<int>(mResolvedShaders.size()))) {
				ImGui::Separator();
				for (const auto& variant : mResolvedShaders) {
					// if (mConstructionArgs && ImGui::Button("Reload")) {
					// Just destroy the variant and evict from the map, easy
					// }
					// else {
						ImGui::Text("%s", variant.second.mVariant.size() ? variant.second.mVariant.c_str() : "No defines");
						ImGui::Separator();
					// }
				}
				ImGui::TreePop();
			}
		}
		else {
			ImGui::Text("Variants (0)");
		}
	}
}
