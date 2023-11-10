#pragma once

#include "Renderer/GLObjects/GLHelper.hpp"

#include <set>
#include <unordered_map>
#include <unordered_set>

namespace neo {
    class ResolvedShaderInstance;

	class NewShader {
    public:
        using ConstructionArgs = std::unordered_map<ShaderStage, std::string>;
        using ShaderSources = std::unordered_map<ShaderStage, const char*>;
        using ShaderDefines = std::unordered_set<std::string>;
        using HashedShaderDefines = HashedString::value_type;

        NewShader(const char* name, const ConstructionArgs& args);
        NewShader(const char* name, const ShaderSources& args);
        ~NewShader();
		NewShader(const NewShader &) = delete;
        NewShader & operator=(const NewShader &) = delete;
        NewShader(NewShader &&) = delete;
        NewShader & operator=(NewShader &&) = delete;

        const ResolvedShaderInstance& getResolvedInstance(const ShaderDefines& defines);
        void imguiEditor();
        void destroy();
    private:
        std::string mName;
        std::optional<ConstructionArgs> mConstructionArgs;
        ShaderSources mShaderSources;
        std::unordered_map<HashedShaderDefines, ResolvedShaderInstance> mResolvedShaders;

        HashedShaderDefines _getDefinesHash(const ShaderDefines& defines);
	};
}