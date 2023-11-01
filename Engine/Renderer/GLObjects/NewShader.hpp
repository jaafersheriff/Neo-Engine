#pragma once

#include "ResolvedShaderInstance.hpp"

namespace neo {

	class NewShader {
    public:
        using ConstructionArgs = std::unordered_map<ShaderStage, const char*>;

        NewShader(const char* name, const ConstructionArgs& args);
        ~NewShader();
		NewShader(const NewShader &) = delete;
        NewShader & operator=(const NewShader &) = delete;
        NewShader(NewShader &&) = delete;
        NewShader & operator=(NewShader &&) = delete;

        ResolvedShaderInstance getResolvedInstance(const ResolvedShaderInstance::ShaderDefines& defines);
    private:
        std::string mName;
        ConstructionArgs mConstructionArgs;
        std::unordered_map<ResolvedShaderInstance::ShaderDefines, ResolvedShaderInstance> mResolvedShaders;
	};
}