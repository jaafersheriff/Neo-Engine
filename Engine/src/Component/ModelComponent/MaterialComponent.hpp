#pragma once

#include "Component/Component.hpp"

#include "Material/Material.hpp"

namespace neo {

    class MaterialComponent : public Component {

        public:
            MaterialComponent(GameObject *go, Material *mat) :
                Component(go),
                mMaterial(mat)
            {}

            const Material & getMaterial() const { return *mMaterial; }

        protected:

            const Material *mMaterial;

    };
}