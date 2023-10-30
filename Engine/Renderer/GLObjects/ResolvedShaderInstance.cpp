#include "Renderer/pch.hpp"

#include "ResolvedShaderInstance.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

namespace neo {
    void ResolvedShaderInstance::bind() const {
        glUseProgram(mPid);
    }

    void ResolvedShaderInstance::unbind() const {
        glUseProgram(0);
    }

    void ResolvedShaderInstance::bindUniform(const char* name, const UniformVariant& uniform) const {
    }

    void ResolvedShaderInstance::bindTexture(const char* name, const Texture& texture) const {
    }
}