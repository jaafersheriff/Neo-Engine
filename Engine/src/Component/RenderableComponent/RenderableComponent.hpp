#pragma once

#include "Component/Component.hpp"

#include "Model/Mesh.hpp"
#include "Model/Material.hpp"

#include <typeindex>

namespace neo {

    class RenderableComponent : public Component {

        public:
            RenderableComponent(GameObject &, Mesh *, Material *);

            virtual void init() override;
            virtual void kill() override;

            template <typename ShaderT> void addShaderType();
            template <typename ShaderT> void removeShaderType();

            virtual const Mesh * getMesh() const { return mesh; }
            virtual const Material * getMaterial() const { return material; }
            void replaceMesh(Mesh *m) { this->mesh = m; }
            void replaceMaterial(Material *m) { this->material = m; }
            const std::vector<std::type_index> & getShaders() { return shaderTypes; }

        protected:
            Mesh * mesh;
            Material * material;
            std::vector<std::type_index> shaderTypes;

        private:
            bool isInit = false;
    };

    /* Template implementation */
    template <typename ShaderT>
    void RenderableComponent::addShaderType() {
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        static_assert(!std::is_same<Shader, ShaderT>::value, "ShaderT must be a derived Shader type");
        std::type_index typeI(typeid(ShaderT));
        auto it = std::find(shaderTypes.begin(), shaderTypes.end(), typeI);
        if (it == shaderTypes.end()) {
            shaderTypes.emplace_back(typeI);
            if (isInit) {
                NeoEngine::template getSystem<RenderSystem>().attachCompToShader(typeI, this);
            }
        }
    }

    template <typename ShaderT>
    void RenderableComponent::removeShaderType() {
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        static_assert(!std::is_same<Shader, ShaderT>::value, "ShaderT must be a derived Shader type");
        std::type_index typeI(typeid(ShaderT));
        auto it = std::find(shaderTypes.begin(), shaderTypes.end(), typeI);
        if (it != shaderTypes.end()) {
            shaderTypes.erase(it);
            if (isInit) {
                NeoEngine::template getSystem<RenderSystem>().detachCompFromShader(typeI, this);
            }
        }
    }
}
