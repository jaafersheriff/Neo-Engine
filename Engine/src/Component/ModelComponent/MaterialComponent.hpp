#pragma once

#include "Component/Component.hpp"

#include "Material/Material.hpp"

namespace neo {

    class MaterialComponent : public Component {

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