#include "RenderableComponent.hpp"

#include "MasterRenderer/MasterRenderer.hpp"

namespace neo {

    void RenderableComponent::init() {
        isInit = true;
        for (auto & shaderT : shaderTypes) {
            MasterRenderer::attachCompToShader(shaderT, this);
        }
    }

    void RenderableComponent::kill() {
        for (auto & shaderT : shaderTypes) {
            MasterRenderer::detachCompFromShader(shaderT, this);
        }
    }

}
