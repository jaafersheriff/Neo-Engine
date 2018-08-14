#pragma once

#include "Component/Component.hpp"

#include "Model/Texture.hpp"

namespace neo {

    class TextureComponent : public Component {

        public:
            TextureComponent(GameObject *go, Texture *tex) :
                Component(go),
                texture(tex)
            {}

            const Texture & getTexture() const { return *texture; }

        protected:

            const Texture *texture;

    };
}
