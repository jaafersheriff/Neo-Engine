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

        private:
            Mesh * mesh;
    };
}
