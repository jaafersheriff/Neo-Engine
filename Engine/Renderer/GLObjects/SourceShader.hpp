#pragma once

#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/ShaderDefines.hpp"

#include <sstream>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace neo {
	class ResolvedShaderInstance;

	class SourceShader {
	public:
		using ConstructionArgs = std::unordered_map<ShaderStage, std::string>;
		using ShaderCode = std::unordered_map<ShaderStage, const char*>;
		using HashedShaderDefines = uint32_t;

		SourceShader(const char* name, const ConstructionArgs& args);
		SourceShader(const char* name, const ShaderCode& args);
		~SourceShader();
		SourceShader(const SourceShader &) = delete;
		SourceShader & operator=(const SourceShader &) = delete;
		SourceShader(SourceShader &&) = delete;
		SourceShader & operator=(SourceShader &&) = delete;

		const ResolvedShaderInstance& getResolvedInstance(const ShaderDefines& defines);
		void imguiEditor();
		void destroy();
	private:
		std::string mName;
		std::optional<ConstructionArgs> mConstructionArgs;
		ShaderCode mShaderSources;
		
		// Can't store ShaderDefines in the map because of const char* and mParent*
		// Also this is faster than specialized std::hash
		std::unordered_map<HashedShaderDefines, ResolvedShaderInstance> mResolvedShaders;
		HashedShaderDefines _getDefinesHash(const ShaderDefines& defines);
	};
}