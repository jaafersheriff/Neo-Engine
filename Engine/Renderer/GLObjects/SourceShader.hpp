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
	class ShaderResourceManager;

	class SourceShader {
		friend ShaderLoader;
		friend ShaderResourceManager;
	public:
		using ConstructionArgs = std::unordered_map<ShaderStage, std::string>;
		using ShaderCode = std::unordered_map<ShaderStage, const char*>;
		using HashedShaderDefines = uint32_t;

		SourceShader(const char* name, const ShaderCode& args);

		const ResolvedShaderInstance& getResolvedInstance(const ShaderDefines& defines) const;
		void destroy();
	private:
		std::string mName;
		std::optional<ConstructionArgs> mConstructionArgs;
		ShaderCode mShaderSources;
		
		// Can't store ShaderDefines in the map because of const char* and mParent*
		// Also this is faster than specialized std::hash
		mutable std::unordered_map<HashedShaderDefines, ResolvedShaderInstance> mResolvedShaders;
		HashedShaderDefines _getDefinesHash(const ShaderDefines& defines) const;
	};
}