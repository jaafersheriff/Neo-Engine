#include "RenderableComponent.hpp"

#include "MasterRenderer/MasterRenderer.hpp"

namespace neo {

    void RenderableComponent::init() {
        mIsInit = true;
        for (auto& shaderT : mShaderTypes) {
            MasterRenderer::attachCompToShader(shaderT, this);
        }
    }

    void RenderableComponent::clearShaderTypes() {
        for (auto& shaderT : mShaderTypes) {
            MasterRenderer::detachCompFromShader(shaderT, this);
        }
    }

    void RenderableComponent::kill() {
        clearShaderTypes();
    }

}
