#pragma once

#include "ResolvedShaderInstance.hpp"

namespace neo {

	class NewShader {
    public:
        enum class ShaderStage {
            VERTEX,
            FRAGMENT,
            GEOMETRY,
            TESSELLATION_CONTROL,
            TESSELLATION_EVAL,
            COMPUTE
        };
        using ConstructionArgs = std::unordered_map<ShaderStage, const char*>;

        NewShader(const ConstructionArgs& args);
        ~NewShader();
		NewShader(const NewShader &) = delete;
        NewShader & operator=(const NewShader &) = delete;
        NewShader(NewShader &&) = delete;
        NewShader & operator=(NewShader &&) = delete;

        ResolvedShaderInstance getResolvedInstance(const ResolvedShaderInstance::ShaderDefines& defines);
    private:
        ConstructionArgs mConstructionArgs;
        std::unordered_map<ResolvedShaderInstance::ShaderDefines, ResolvedShaderInstance> mResolvedShaders;
	};
}