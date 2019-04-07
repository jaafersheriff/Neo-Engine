#pragma once

#include "Component/Component.hpp"

#include "GLHelper/Texture.hpp"

// TODO : template class..
namespace neo {

    class DiffuseMapComponent : public Component {

        public:
            DiffuseMapComponent(GameObject *go, Texture *tex) :
                Component(go),
                mTexture(tex)
            {}

            const Texture *mTexture;

    };

    class SpecularMapComponent : public Component {

        public:
            SpecularMapComponent(GameObject *go, Texture *tex) :
                Component(go),
                texture(tex)
            {}

            const Texture *texture;

    };

    class NormalMapComponent : public Component {

        public:
            NormalMapComponent(GameObject *go, Texture *tex) :
                Component(go),
                texture(tex)
            {}

            const Texture *texture;

    };

    class CubeMapComponent : public Component {

        public:
            CubeMapComponent(GameObject *go, Texture *tex) :
                Component(go),
                texture(tex)
            {}

            const Texture *texture;

    };
}
