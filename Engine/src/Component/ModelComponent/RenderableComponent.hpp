#pragma once

#include "Component/Component.hpp"

#include "GLHelper/Mesh.hpp"

#include <typeindex>

namespace neo {

    class RenderableComponent : public Component {

        public:
            RenderableComponent(GameObject *go, Mesh *m) :
                Component(go),
                mesh(m)
            {}

            virtual void init() override;
            virtual void kill() override;

            template <typename ShaderT> void addShaderType();
            template <typename ShaderT> void removeShaderType();

            virtual const Mesh & getMesh() const { return *mesh; }
            void replaceMesh(Mesh *m) { this->mesh = m; }
            const std::vector<std::type_index> & getShaders() { return shaderTypes; }

        protected:
            Mesh * mesh;
            std::vector<std::type_index> shaderTypes;

        private:
            bool isInit = false;
    };

    /* Template implementation */
    template <typename ShaderT>
    void RenderableComponent::addShaderType() {
        static_assert(std::is_base_of<Shader, ShaderT>::value, "ShaderT must be a Shader type");
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        std::type_index typeI(typeid(ShaderT));
        auto it = std::find(shaderTypes.begin(), shaderTypes.end(), typeI);
        if (it == shaderTypes.end()) {
            shaderTypes.emplace_back(typeI);
            if (isInit) {
                MasterRenderer::attachCompToShader(typeI, this);
            }
        }
    }

    template <typename ShaderT>
    void RenderableComponent::removeShaderType() {
        static_assert(std::is_base_of<Shader, ShaderT>::value, "ShaderT must be a Shader type");
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        std::type_index typeI(typeid(ShaderT));
        auto it = std::find(shaderTypes.begin(), shaderTypes.end(), typeI);
        if (it != shaderTypes.end()) {
            shaderTypes.erase(it);
            if (isInit) {
                MasterRenderer::detachCompFromShader(typeI, this);
            }
        }
    }
}
