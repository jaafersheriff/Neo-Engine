#pragma once

#include "Renderer/GLObjects/GLHelper.hpp"

#include <set>
#include <unordered_map>

namespace neo {
    class ResolvedShaderInstance;

	class NewShader {
    public:
        using ConstructionArgs = std::unordered_map<ShaderStage, const char*>;
        using ShaderDefines = std::set<std::string>;
        using HashedShaderDefines = HashedString::value_type;

        NewShader(const char* name, const ConstructionArgs& args);
        ~NewShader();
		NewShader(const NewShader &) = delete;
        NewShader & operator=(const NewShader &) = delete;
        NewShader(NewShader &&) = delete;
        NewShader & operator=(NewShader &&) = delete;

        const ResolvedShaderInstance& getResolvedInstance(const ShaderDefines& defines);
    private:
        std::string mName;
        ConstructionArgs mConstructionArgs;
        std::unordered_map<HashedShaderDefines, ResolvedShaderInstance> mResolvedShaders;

        HashedShaderDefines _getDefinesHash(const ShaderDefines& defines);
	};
}