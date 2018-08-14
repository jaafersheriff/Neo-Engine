#pragma once

#include "Component/Component.hpp"

#include "Model/Texture.hpp"

namespace neo {

    class TextureComponent : Component {

        public:
            TextureComponent(GameObject *go, Texture *mat) :
                Component(go),
                material(mat)
            {}

            const Texture & getTexture() const { return *material; }

        protected:

            const Texture *material;

    };
}
