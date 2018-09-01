#pragma once

#include "Component/Component.hpp"

#include "GLHelper/Texture.hpp"

namespace neo {

    class SpecularMapComponent : public Component {

        public:
            SpecularMapComponent(GameObject *go, Texture *tex) :
                Component(go),
                texture(tex)
            {}

            const Texture & getTexture() const { return *texture; }

        protected:

            const Texture *texture;

    };
}
