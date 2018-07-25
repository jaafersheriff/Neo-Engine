#pragma once

#include "Component/Component.hpp"

#include "Model/Mesh.hpp"

namespace neo {

    class RenderSystem;

    class RenderableComponent : public Component {

        friend RenderSystem;

        public:
            RenderableComponent(GameObject &, Mesh *);

            virtual void kill() override;

            bool addShaderType(std::type_index shaderT);
            void removeShaderType(std::type_index shaderT);

            const Mesh *getMesh() const { return mesh; }
            void replaceMesh(Mesh *m) { this->mesh = m; }
            const std::vector<std::type_index> & getShaders() { return shaderTypes; }

        private:
            Mesh * mesh;
            std::vector<std::type_index> shaderTypes;
    };

}
