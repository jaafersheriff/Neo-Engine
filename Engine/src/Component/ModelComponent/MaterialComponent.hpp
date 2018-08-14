#pragma once

#include "Component/Component.hpp"

#include "Model/Material.hpp"

namespace neo {

    class MaterialComponent : Component {

        public:
            MaterialComponent(GameObject *go, Material *mat) :
                Component(go),
                material(mat)
            {}

            const Material & getMaterial() const { return *material; }

        protected:

            const Material *material;

    };
}