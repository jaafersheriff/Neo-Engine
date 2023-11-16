#pragma once

#include "Renderer/GLObjects/GLHelper.hpp"

#include <set>
#include <unordered_map>
#include <unordered_set>

namespace neo {
    class ResolvedShaderInstance;

	class SourceShader {
    public:
        using ConstructionArgs = std::unordered_map<ShaderStage, std::string>;
        using ShaderCode = std::unordered_map<ShaderStage, const char*>;
        using ShaderDefines = std::unordered_set<std::string>;
        using HashedShaderDefines = HashedString::value_type;

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
        std::unordered_map<HashedShaderDefines, ResolvedShaderInstance> mResolvedShaders;

        HashedShaderDefines _getDefinesHash(const ShaderDefines& defines);
	};
}