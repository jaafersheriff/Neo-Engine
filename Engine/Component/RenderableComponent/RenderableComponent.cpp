#include "RenderableComponent.hpp"

namespace neo {

    RenderableComponent::RenderableComponent(GameObject &go, Mesh *m) :
        Component(go),
        mesh(m)
    {}

    void RenderableComponent::kill() {
        // RenderSystem & rSystem = NeoEngine::getSystem<RenderSystem>();
        // for (auto shaderT : shaderTypes) {
        // }
    }

    bool RenderableComponent::addShaderType(std::type_index shaderT) {
        auto it = std::find(shaderTypes.begin(), shaderTypes.end(), shaderT);
        if (it == shaderTypes.end()) {
            shaderTypes.emplace_back(shaderT);
            return true;
        }
        return false;
    }

    void RenderableComponent::removeShaderType(std::type_index shaderT) {
        auto it = std::find(shaderTypes.begin(), shaderTypes.end(), shaderT);
        if (it != shaderTypes.end()) {
            shaderTypes.erase(it);
        }
    }

}
