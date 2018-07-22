#pragma once

#include "Component/Component.hpp"

#include "Model/Mesh.hpp"

namespace neo {

    class RenderableComponent : public Component {

        public:
            RenderableComponent(GameObject &go, Mesh *m) :
                Component(go),
                mesh(m) 
            {}

            const Mesh *getMesh() const { return mesh; }
            void replaceMesh(Mesh *m) { this->mesh = m; }
            const std::vector<std::type_index> & getShaders() { return shaderTypes; }

            bool addShaderType(std::type_index shaderT) {
                auto it = std::find(shaderTypes.begin(), shaderTypes.end(), shaderT);
                if (it == shaderTypes.end()) {
                    shaderTypes.emplace_back(shaderT);
                    return true;
                }
                return false;
            }

            void removeShaderType(std::type_index shaderT) {
                auto it = std::find(shaderTypes.begin(), shaderTypes.end(), shaderT);
                if (it != shaderTypes.end()) {
                    shaderTypes.erase(it);
                }
            }

        private:
            Mesh * mesh;
            std::vector<std::type_index> shaderTypes;
    };

}
