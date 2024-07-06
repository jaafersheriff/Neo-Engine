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
	struct ShaderLoader;
	class ShaderManager;

	class SourceShader {
		friend ShaderLoader;
		friend ShaderManager;
	public:
		using ConstructionArgs = std::unordered_map<types::shader::Stage, std::string>;
		using ShaderCode = std::unordered_map<types::shader::Stage, const char*>;
		using HashedShaderDefines = uint32_t;

		SourceShader(const char* name, const ShaderCode& args);

		const ResolvedShaderInstance& getResolvedInstance(const ShaderDefines& defines) const;
		void destroy();
	private:
		std::string mName;
		std::optional<ConstructionArgs> mConstructionArgs;
		time_t mModifiedTime;
		ShaderCode mShaderSources;
		
		// Can't store ShaderDefines in the map because of const char* and mParent*
		// Also this is faster than specialized std::hash
		mutable std::unordered_map<HashedShaderDefines, ResolvedShaderInstance> mResolvedShaders;
		HashedShaderDefines _getDefinesHash(const ShaderDefines& defines) const;
	};
}